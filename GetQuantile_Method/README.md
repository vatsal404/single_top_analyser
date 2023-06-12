GetQuantile Method:
Quantiles_jj.C:
1- Create 2D plot :
    -X-axis is the main variable we want to study 
    -Y-axis is the variable we want to cut on

2- Projecting a single Y-axis bin for each X-axis bin
    -design criteria to retain a given percentage of the input distribution.
    -determine the quantiles for each cut variable  in order to have the same efficiency as the cut based analysis.



MergedBins.C :
How to Merge Bins: 
1- Create 2D plot as explained above!

2- Projecting a single Y-axis bin for each X-axis bin
    -Check the entries of each Histogram
    -Look for Entries <500 for merging the bins
    -Merging the bins  if the Entries less than 500 
    

