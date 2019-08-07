#include "input.h"
#include "experimentinput.h"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <fstream>

ExperimentInput::ExperimentInput(Chrono* newChrono, const Config* newConfig): Input(newChrono, newConfig)
{
    // set the signal amplitudes to 1 and change
    // the noise variance to account for SNR
    signalMagnitude = 1;
    noiseStdDeviation = pow(10,-config->getSNRdB()/20);
}

ExperimentInput::~ExperimentInput()
{
}

void ExperimentInput::process()
{
    chrono->start("Input");
    
    for (int k = 0; k < config->getSignalLength(); k++)
    {
	   neededSamples.insert(k);
    }
    
    generateNonZeroFrequencies();
    frequencyToTime();
    
    if (config->isNoisy())
    {
        addNoise();
    }
    
    for (auto t = neededSamples.cbegin(); t != neededSamples.cend(); ++t)
    {
        timeSignal[*t] /= sqrt((ffast_complex)config->getSignalLengthOriginal());
    }
    
    chrono->stop("Input");
}

void ExperimentInput::process(std::vector<int> delays)
{
    chrono->start("Input");
    
    findNeededSamples(delays);

    generateNonZeroFrequencies();
    
    frequencyToTime();
    

    if (config->isNoisy())
    {
        addNoise();
    }
    for (auto t = neededSamples.cbegin(); t != neededSamples.cend(); ++t)
    {
        timeSignal[*t] /= sqrt((ffast_complex)config->getSignalLengthOriginal());
    }
    
    chrono->stop("Input");
}

const std::unordered_map<int,ffast_complex>& ExperimentInput::getTimeSignal() const
{
    return timeSignal;
}

ffast_complex ExperimentInput::getSignalAtIndex(int k) const 
{
    return timeSignal.at(k);
}

const std::unordered_map<int, ffast_complex> &ExperimentInput::getNonZeroFrequencies() const
{
    return nonZeroFrequencies;
}

ffast_real ExperimentInput::getSignalMagnitude() const
{
    return signalMagnitude;
}

ffast_real ExperimentInput::getRealSNR() const
{
    return realSNR;
}

ffast_real ExperimentInput::getRealSNRdB() const
{
    return (ffast_real) 10*log10((double) realSNR);
}

ffast_real ExperimentInput::distribution(ffast_real _urand, std::vector<double> F) 
{
    int l = F.size();
    for (int i = 1; i < l; ++i)
    {
        if(_urand < F[i])
            return ( ( (_urand-F[i])/(F[i]-F[i-1]) + ( (double) i) )/( (double) l-1.0) );
    }
    return 1;
}

void ExperimentInput::generateNonZeroFrequencies()
{
    std::set<int> tempLocations;
    nonZeroFrequencies.clear();

    while((int) tempLocations.size() < config->getSignalSparsity())
    {
        int tempLocation = ((int) floor(config->getSignalLengthOriginal()*distribution((ffast_real) (double(rand())/RAND_MAX), config->getDistribution())))
                % config->getSignalLengthOriginal();
	
	// for off-grid we need guard bands
	if (config->getSignalLengthOriginal() != config->getSignalLength() ) 
	{
            if (tempLocations.count(tempLocation-5)+
		tempLocations.count(tempLocation-4)+
	        tempLocations.count(tempLocation-3)+
		tempLocations.count(tempLocation-2)+
		tempLocations.count(tempLocation-1)+
		tempLocations.count(tempLocation+0)+
		tempLocations.count(tempLocation+1)+
		tempLocations.count(tempLocation+2)+
		tempLocations.count(tempLocation+3)+
		tempLocations.count(tempLocation+4)+
		tempLocations.count(tempLocation+5) == 0)
	    {
		tempLocations.insert(tempLocation);
	    }
	}
	else
	{
	    tempLocations.insert(tempLocation);
	}
    }

    for(auto it = tempLocations.cbegin(); it != tempLocations.cend(); ++it)
    {
        nonZeroFrequencies[*it] = std::polar(signalMagnitude, getRandomPhase());
    }
}

ffast_real ExperimentInput::getRandomPhase() const
{
    if (config->getPhasesNb() < 1)
    {
       return 2*M_PI*(double(rand())/RAND_MAX);
    }
    else if (config->getPhasesNb() == 1)
    {
        return 0;
    }
    else if (config->getPhasesNb() == 2)
    {
        return M_PI*std::round((double(rand())/RAND_MAX));
    }
    else
    {
        return (std::floor(config->getPhasesNb()*(double(rand())/RAND_MAX))*2 + 1)*M_PI/config->getPhasesNb();
    }
}

void ExperimentInput::addNoise()
{
    ffast_real signalPower = signalMagnitude * signalMagnitude;
    ffast_real noisePower = 0;
    ffast_real noiseNormFactor;
    ffast_real noisePhase;

    for (auto t = neededSamples.cbegin(); t != neededSamples.cend(); ++t)
    {
        noiseNormFactor = (ffast_real) -2*log((double(rand())/RAND_MAX));
        noisePhase = (ffast_real) 2*M_PI*(double(rand())/RAND_MAX);
        noisePower += noiseNormFactor; // Standard deviation added later.
        timeSignal[*t] += std::polar( noiseStdDeviation * ((ffast_real) sqrt( (double) noiseNormFactor ) ) / sqrt(2) , noisePhase );
    }

    noisePower *= noiseStdDeviation * noiseStdDeviation;
    realSNR = signalPower * config->getSignalLength()/noisePower;
}

void ExperimentInput::frequencyToTime()
{
    ffast_real w_0 = 2*M_PI/config->getSignalLengthOriginal();
    ffast_real w_0t;

    // go through the required sample indices
    for (auto t = neededSamples.cbegin(); t != neededSamples.cend(); ++t)
    {
        timeSignal[*t] = 0;
        w_0t = w_0*(*t);

	// go through the non-zero frequency components
        for (auto f = nonZeroFrequencies.cbegin(); f != nonZeroFrequencies.cend(); ++f)
        {
            timeSignal[*t] += (f->second)*std::polar( 1.0,(ffast_real) (w_0t*(f->first)) );
        }
    }
}

void ExperimentInput::findNeededSamples(std::vector<int> delays)
{
    int stageJumpFactor;

    // if the problem is off-grid    
    if ( config->getSignalLengthOriginal() != config->getSignalLength() )
    {
        for (int i = 0; i < config->getSignalSparsity()*100; ++i)
        {
            neededSamples.insert(i);
        }
    }

    // go over each stage
    for (int stage=0; stage<config->getBinsNb(); stage++)
    {
        stageJumpFactor = config->getSignalLength()/config->getBinSize(stage);

        // go over each delay
        for(auto delayIterator=delays.begin(); delayIterator != delays.end(); ++delayIterator)
        {
            // delay within the bin
            for (int binRelativeIndex=0; binRelativeIndex<config->getBinSize(stage); binRelativeIndex++)
            {
                neededSamples.insert((int) (*delayIterator + binRelativeIndex * stageJumpFactor) % config->getSignalLength());

            }
        }
    }
}
