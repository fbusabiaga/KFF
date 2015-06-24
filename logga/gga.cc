#include <stdio.h>
#include <stdlib.h>

#include "gga.h"
#include "population.h"
#include "statistics.h"
#include "replace.h"
#include "select.h"
#include "graph.h"
#include "fitness.h"
#include "memalloc.h"
#include "random.h"

// various output files

FILE *logFile;
FILE *fitnessFile;
FILE *modelFile;

BasicStatistics populationStatistics;


// the description of termination criteria that are checked

char *terminationReasonDescription[5] = {
"No reason",
"Maximal number of generations reached",
"Solution convergence (with threshold epsilon)",
"Proportion of optima in a population reached the threshold",
"Optimum has been found"};


int GGA(GGAParams *ggaParams)
{
  long       N,numOffspring,numParents,t;
  // set sizes of populations
  N            = ggaParams->N;
  numOffspring = (long) ((float) ggaParams->N*ggaParams->percentOffspring)/100;
  numParents   = (long) ggaParams->N;

  Population population(N),parents(numParents),offspring(numOffspring);
  int        terminationReason;

  // initialize the poplulation according to parameters
  population.Initialize(ggaParams);

  // randomly generate first population according to uniform distribution
  population.GeneratePopulation();

  // evaluate first population
  population.EvaluatePopulation();

  // main loop
  t=0;

  // compute basic statistics on initial population
  ComputeBasicStatistics(&populationStatistics,t,&population,ggaParams);

  // output the statistics on first generation
  GenerationStatistics(stdout,&populationStatistics);
  GenerationStatistics(logFile,&populationStatistics);
  FitnessStatistics(fitnessFile,&populationStatistics);

  // pause after statistics?
  Pause(ggaParams);

  while (!(terminationReason=TerminationCriteria(ggaParams)))
    {
      // perform truncation (block) selection
      SelectTheBest(&population,&parents,ggaParams);

      // create offspring
      GenerateOffspring(&parents,&offspring,ggaParams);
      
      // evaluate the offspring
      offspring.EvaluatePopulation();
      
      // replace the worst of the population with offspring
      ReplaceWorst(&population,&offspring);
      
      // increase the generation number
  
      t++;

      // compute basic statistics
      ComputeBasicStatistics(&populationStatistics,t,&population,ggaParams);

      // output the statistics on current generation
      GenerationStatistics(stdout,&populationStatistics);
      GenerationStatistics(logFile,&populationStatistics);
      FitnessStatistics(fitnessFile,&populationStatistics);

      // pause after statistics?
      Pause(ggaParams);
    };

  // print out final statistics
  ComputeBasicStatistics(&populationStatistics,t,&population,ggaParams);
  
  FinalStatistics(stdout,terminationReasonDescription[terminationReason],&populationStatistics);
  FinalStatistics(logFile,terminationReasonDescription[terminationReason],&populationStatistics);
  
  return 0;
}


int TerminationCriteria(GGAParams *ggaParams)
{
  int result;

  // no reason to finish yet

  result=0;

  // check if the proportion of optima reached the required value, if yes terminate
  if ((!result)&&(ggaParams->maxOptimal>=0))
    result = (float(populationStatistics.numOptimal*100)/(float)populationStatistics.N>=ggaParams->maxOptimal)? MAXOPTIMAL_TERMINATION:0;

  // check if should terminate if optimum has been found and if this is the case if yes
  if ((!result)&&(ggaParams->stopWhenFoundOptimum))
    if (IsBestDefined())
      result = (IsOptimal(populationStatistics.bestX,populationStatistics.n))? OPTIMUMFOUND_TERMINATION:0;

  // if there's no reason to finish yet and the epsilon threshold was set, check it
  if ((!result)&&(ggaParams->epsilon>=0))
    {
      int   k;
      float epsilon1;

      // set epsilon1 to (1-ggaParams->epsilon)
      epsilon1 = 1-ggaParams->epsilon;

      // are all frequencies closer than epsilon to either 0 or 1?
      result=EPSILON_TERMINATION;
      for (k=0; k<populationStatistics.n; k++)
	if ((populationStatistics.p1[k]>=ggaParams->epsilon)&&(populationStatistics.p1[k]<=epsilon1))
	  result=0;
    }

  // check if the number of generations wasn't exceeded
  if ((!result)&&(ggaParams->maxGenerations>=0))
    result = (populationStatistics.generation>=ggaParams->maxGenerations)? MAXGENERATIONS_TERMINATION:0;

  return result;
}

/*
int GenerateOffspring(long iteration, Population *parents, Population *offspring, GGAParams *ggaParams)
{

  // apply cross over  

  // apply mutation at random

  return 0;
}
*/

int Pause(GGAParams *ggaParams)
{
  if (ggaParams->pause)
    {
      printf("Press Enter to continue.");
      getchar();
    };

  return 0;
}


int Terminate(GGAParams *ggaParams)
{
  // get rid of the metric

  //doneMetric();

  // get rid of the fitness

  doneFitness(ggaParams);

  // statistics done

  DoneBasicStatistics(&populationStatistics);

  // close output streams

  if (logFile)
    fclose(logFile);

  if (fitnessFile)
    fclose(fitnessFile);
 
 if (modelFile)
    fclose(modelFile);

  return 0;  
}


FILE *GetLogFile()
{
  return logFile;
}

FILE *GetModelFile()
{
  return modelFile;
}

FILE *GetFitnessFile()
{
  return fitnessFile;
}
