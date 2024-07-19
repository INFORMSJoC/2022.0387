![INFORMS Journal on Computing Logo](https://INFORMSJoC.github.io/logos/INFORMS_Journal_on_Computing_Header.jpg)

# A Novel Prediction Setup for Online Speed-Scaling

This archive is distributed in association with the [INFORMS Journal on Computing](https://pubsonline.informs.org/journal/ijoc) under the [MIT License](LICENSE).

The software and data in this repository are a snapshot of the software and data that were used in the research reported on in the paper [A Novel Prediction Setup for Online Speed-Scaling](https://doi.org/10.1287/ijoc.2023.0257) by A. Antoniadis, P. Jabbarzade, and G. Shahkarami.

## Cite

To cite the contents of this repository, please cite both the paper and this repo, using their respective DOIs.

https://doi.org/10.1287/ijoc.2022.0387

https://doi.org/10.1287/ijoc.2022.0387.cd

Below is the BibTex for citing this snapshot of the repository.

```
@misc{FengRMA2024,
  author =        {A. Antoniadis, P. Jabbarzade, and G. Shahkarami},
  publisher =     {INFORMS Journal on Computing},
  title =         {A Novel Prediction Setup for Online Speed-Scaling},
  year =          {2024},
  doi =           {10.1287/ijoc.2022.0387.cd},
  url =           {https://github.com/INFORMSJoC/2022.0387},
  note =          {Available for download at https://github.com/INFORMSJoC/2022.0387},
}  
```

## Description

The goal of this repository is to demonstrate the efficiency of our algorithm for scheduling jobs with predictions.

To run the experiments, we use data from [this source](https://ita.ee.lbl.gov/html/contrib/EPA-HTTP.html). 
For more details, refer to the Data section below. 
Additionally, we utilize code from the paper "An Experimental Comparison of Speed Scaling Algorithms with Deadline Feasibility Constraints" by A. Abousamra, D.P. Bunde, and K. Pruhs, available [here](https://people.cs.pitt.edu/~kirk/SpeedScalingExperiments/), to generate jobs from the raw data and compare our algorithm with others. 
It is crucial to download their code to replicate the experiments.

## Data

Our experiments utilize the [epa-http](data/epa-http.txt) trace file from the [Internet Traffic Archive](http://ita.ee.lbl.gov/), contains a day's worth of HTTP requests received by the Environmental Protection Agency's web server at Research Triangle Park, North Carolina. 
The data was collected by Laura Bottomley.
More information about the data can be found [here](https://ita.ee.lbl.gov/html/contrib/EPA-HTTP.html).

## Generate Instance

The file [epa-http](data/epa-http.txt) contains raw data, from which we need to generate jobs. 
We use PrepareInput, implemented by [Abousamra et al.](https://people.cs.pitt.edu/~kirk/SpeedScalingExperiments/), with a slight modification:

At line 838, add the following code:
    
    printf("%d\n", lActualNumberOfJobs);
    for (long lIndex = 0; lIndex < lActualNumberOfJobs; lIndex++)
    {
            printf("%f %f %d\n", pstrctJIJobs[lIndex].dReleaseTime, pstrctJIJobs[lIndex].dDeadline, pstrctJIJobs[lIndex].lWork);
    }

This modification helps us obtain the job data in a human-readable format. 
We use the following command to generate 1000 jobs after compiling the modified code using their makefile:

    ./PrepareInput 0 4 1 1000 data/epa-http.txt jobs 10 > jobs-human-readable

The `jobs` file can be used as input for running YDS and qOA, which are explained in the Running Algorithms section. 
To generate predictions for jobs, which are crucial for running our algorithm, use the [generate_precition](src/generate_prediction.cpp) script.
The [generate_precition](src/generate_prediction.cpp) script takes the argument `<stddev>`, which is the standard deviation of the normal distribution used to generate predictions.
In our paper, we set `stddev={0.01, 0.05, 0.1}`.

    g++ src/generate_prediction.cpp -o generate_prediction
    ./generate_prediction <stddev> < jobs-human-readable > jobs-with-prediction

## Running Algorithms

Here, we demonstrate how to run our algorithm, the optimal offline algorithm YDS, and the online algorithm qOA, implemented by[Abousamra et al.](https://people.cs.pitt.edu/~kirk/SpeedScalingExperiments/).
Note that to run the YDS and qOA algorithms, in addition to the `YDS` and `qOA` modules, you will need the `ComputeEnergy` module from [Abousamra et al.](https://people.cs.pitt.edu/~kirk/SpeedScalingExperiments/) to compute the energy consumption of these algorithms.
Do not forget to compile their modules using the makefile they provided.

### Our Algorithm
The [SwP](src/SwP.cpp) algorithm takes two arguments: `<lambda>` and `<mu>`.
More details on how these parameters affect the algorithm can be found in our paper.
Note that we set `lambda={0, 0.1, 0.2, 0.3}` and `mu={0.1, 0.2, ···, 1}` to examine our algorithm.

Compile and run the [SwP](src/SwP.cpp) code:

    g++ src/SwP.cpp -o SwP
    ./SwP <lambda> <mu> < jobs-with-prediction 

The output is the energy consumption of our scheduling algorithm for the given input.

### YDS Algorithm
Run YDS with the following command:

    ./YDS C jobs 1000 yds-output
    ./ComputeEnergy yds-output yds-energy-consumption 3

### qOA Algorithm
Run qOA with `q=1.667`, which is theoretically optimal for the worst case:

    ./qOA jobs 1000 qoa-output 1.667
    ./ComputeEnergy qoa-output qoa-energy-consumption 3
