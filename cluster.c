/* File cluster.c */
/* ANA routines dealing with cluster analysis */
/* Louis Strous / started 18 August 1995 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>		/* for DBL_MAX */
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: cluster.c,v 4.0 2001/02/07 20:36:57 strous Exp $";

int	ana_replace(int, int);
void	randomu(int seed, void *output, int number, int modulo);
/*----------------------------------------------------------------*/
int fptrCompare(const void *p1, const void *p2)
     /* auxilliary function for qsort call in ana_cluster */
{
  if (**(double **) p1 < **(double **) p2) return -1;
  if (**(double **) p1 > **(double **) p2) return 1;
  return 0;
}
/*----------------------------------------------------------------*/
int ana_cluster(int narg, int ps[])
/* CLUSTER, DATA [, CENTERS=c, INDEX=i, SIZE=sz, SAMPLE=s, PHANTOM=p,
   MAXIT=m, RMS=r, /UPDATE, /ITERATE, /VOCAL, /QUICK]

 divides all vectors DATA(*,...) into clusters, based on their
 proximity to cluster centers.  Each data point is assigned to the
 cluster whose center is closest of all cluster centers to the data
 point at that time.  <DATA>, <CENTERS>, <INDEX>, <SAMPLE>, <PHANTOM>,
 and <MAXIT> are input variables; <CENTERS>, <INDEX>, and <SIZE> are
 output variables and must therefore be named variables.

 If <CENTERS> is a scalar on entry, then it denotes the number of
 clusters to divide the data into, and a random sample (of appropriate
 size) of the data vectors is taken as initial guess for the cluster
 center positions.  If <CENTERS> is an array on input, then each
 CENTERS(*,k...) is taken to contain the (initial) position of a
 cluster center.  The final (possibly updated) cluster centers are
 returned in <CENTERS> on exit.

 If <INDEX> is an array of appropriate size on entry, and if <PHANTOM>
 is not specified, then the elements of <INDEX> denote the initial
 cluster membership of the datapoints.  On exit, <INDEX> contains the
 cluster numbers that the data points have been assigned to.

 If <SIZE> is specified, then the number of elements in each cluster
 is returned in it upon exit from the routine.

 If <SAMPLE> is an integer larger than one, then it indicates the size
 of a random sample of data points that should be treated.  If
 <SAMPLE> equals 1, then a sample ten times bigger than the number of
 clusters is used.  If <SAMPLE> is not specified or falls outside the
 range mentioned before, then all data points are treated.

 If <PHANTOM> is specified, then it indicates that the clusters should
 be pre-stocked with phantom members.  The value assigned to
 <PHANTOM>, if an integer larger than 1, indicates how many phantom
 members to assign to each cluster prior to treatment of the data
 points, to partially suppress movement of the cluster centers during
 clustering.  If <PHANTOM> is not specified, and <INDEX> is an array
 of appropriate size, then the clusters contain the members indicated
 by <INDEX> prior to clustering.  If <PHANTOM> equals 1, then 10
 phantom members are assigned to each cluster prior to clustering.
 Any phantom members are removed after clustering, before exiting the
 routine.

 <MAXIT> specifies the maximum number of iterations that is allowed
 for this call to CLUSTER.  If <MAXIT> is not specified, then the
 number of iterations is unlimited.

 If <RMS> is specified then the average root-mean-square distance of
 the members of each cluster to its center is returned in it.

 Keyword /UPDATE signals that the cluster center positions must be
 updated during the clustering, so that each cluster center at any
 time during the clustering equals the average position of all members
 in the cluster at that time (including any phantom members).  If
 /UPDATE is not specified, then the cluster centers do not move during
 clustering.

 Keyword /ITERATE specifies that updating must be iterated until the
 cluster centers are stable and upon exit all data points are members
 of the cluster whose center is the closest one of all cluster
 centers.

 Keyword /VOCAL specifies that information about the number of
 reclustered data points and the number of clusters that have had
 members reclustered (either into or out of the current cluster) is
 displayed after each iteration.

 Keyword /QUICK specifies that only data points in clusters that were
 changed during the last iteration or so far during the current one
 should be treated during the current iteration.  The other data
 points are left unchanged.

 /ITERATE implies /UPDATE.  No <INDEX> implies /PHANTOM.  <INDEX> is
 ignored if <SAMPLE> implies that not all data points are treated.
 /QUICK implies /ITERATE and /UPDATE. */

/* Louis Strous 18aug95 14oct95 20sep96*/

/* Strategy: */
/* 1. choose initial cluster centers as a random sample from the */
/* data.  2. determine the direction of greatest variation (DGV) from the */
/* cluster centers.  3. calculate the projections of the cluster */
/* centers on the DGV (PDGV).  4. order the cluster centers according to */
/* their PDGV.  5. choose a random sample of data points to treat.  6. for */
/* each of the selected data points, calculate its PDGV.  7. find the */
/* cluster center with its PDGV closest to the data point's PDGV. */
/* 8. calculate the total distance of that data point to that cluster */
/* center.  All cluster centers with a PDGV further away from the data */
/* point's PDGV than the data point's distance to the selected cluster */
/* center need not be considered anymore for this data point.  9. for */
/* all remaining cluster centers, calculate their distance to the data */
/* point.  update the closest cluster center and the PDGV condition */
/* until the closest cluster center has been found. */
{ 
  void	random_unique(int seed, int *output, int number, int modulo);
  int	iq, nClusters, nVectorDim, nVectors, i, j, *index, size, dataIndex;
  float	*data, *dataPoint, n1, n2, f;
  double	s, t, s2, dMin2, d, *center, *center2, *group1, *group2,
	*dgv, *pdgv, *centroid, *fp, **findex, *firstCenter, *scrap,
	*rmsptr;
  int	nSample, *clusterSize, k, n, l, k2, m, dims[2], k0,
	kBest, *dataDims, nDataDims, *clusterCtoO, *clusterOtoC, nChanged,
	nIter, curO, phantom, newO, *iPtr, maxit, rms;
  char	gotIndex, gotSize, gotSample, update, gotCenter,
	useIndex, iterate, *changed, *changedOld, vocal, ordered,
	gotPhantom, recluster, quick, record, curChanged;
  byte	indexType;
  pointer	clusterNumber, clusterNumber2;
  FILE	*file;
  int	nDistCal = 0, allDistCal = 0;

  /* 0. Initialization */
  if (ps[1] >= NAMED_END)	/* CENTERS is not a named variable */
    return anaerror("Argument must be a named variable", ps[1]);
  iq = ana_float(1, ps);	/* the data (ensure ANA_FLOAT) */
  if (symbol_class(iq) != ANA_ARRAY) /* data is not an array */
    return cerror(NEED_ARR, *ps);
  dataDims = array_dims(iq);	/* data dimensions */
  nDataDims = array_num_dims(iq); /* # data dimensions */
  nVectorDim = dataDims[0]; /* # dimensions in each data vector */
  nVectors = array_size(iq)/nVectorDim; /* # vectors */
  data = (float *) array_data(iq); /* data points */
  if (narg >= 4 && ps[3]) {	/* SIZE */
    gotSize = 1;
    if (ps[3] >= NAMED_END)
      return anaerror("Output argument must be a named variable", ps[3]);
  } else
    gotSize = 0;
  if (narg >= 5 && ps[4]) {	/* SAMPLE */
    gotSample = 1;
    nSample = int_arg(ps[4]);
  } else
    gotSample = 0;

  if (narg >= 6 && ps[5]) {	/* PHANTOM */
    gotPhantom = 1;
    phantom = int_arg(ps[5]);
    if (phantom < 0)
      return anaerror("Number of phantom members must be >= 0", ps[5]);
    else if (ps[5] == ANA_ONE)	/* /PHANTOM */
      phantom = 10; 		/* default number */
  } else {
    gotPhantom = 0;
    phantom = 0;
  }
  if (narg >= 3 && ps[2])	/* INDEX */
    gotIndex = 1;
  else
    gotIndex = 0;

  if (narg >= 7 && ps[6]) {	/* MAXIT */
    maxit = int_arg(ps[6]);
    if (maxit <= 0)
      return anaerror("Maximum number of iterations must be positive", ps[6]);
  } else
    maxit = 0;

  if (narg >= 8 && ps[7]) {	/* RMS */
    if (ps[7] >= NAMED_END)
      return anaerror("Output argument must be a named variable", ps[7]);
    rms = ps[7];
  } else
    rms = 0;

  useIndex = gotIndex;
  /* 1. Determine initial cluster centers */
  k = nVectors;
  if (gotSample && k > nSample && nSample > 1)
    k = nSample;
  size = sizeof(double)*nVectorDim; /* size in bytes of one data point */
  switch (symbol_class(ps[1])) { /* CENTERS class */
    case ANA_SCALAR:		/* the scalar denotes the number of */
      /* clusters to find.  Get random sample of data vectors to act */
      /* as (initial) cluster centers */
      gotCenter = 0;		/* signal that the user did not specify */
				/* cluster centers */
      nClusters = int_arg(ps[1]); /* # clusters to find */
      if (nClusters < 2 || nClusters >= k) /* wrong number requested */
	return anaerror("Number of clusters must lie between 2 and %1d",
		     ps[1], k);
      dims[0] = nVectorDim;	/* # dimensions per data point */
      dims[1] = nClusters;	/* # clusters */
      redef_array(ps[1], ANA_DOUBLE, 2, dims);
      center = (double *) array_data(ps[1]); /* cluster centers */

      if (gotSample) {
	if (nSample == 1) {
	  nSample = nVectors/10;
	  if (nSample < 10*nClusters)
	    nSample = 10*nClusters;
	}
	if (nSample < 1 || nSample > nVectors)
	  nSample = nVectors;
      } else
	nSample = nVectors;
      if (nSample == nVectors)
	gotSample = 0;		/* sample selects all */

      /* get nClusters numbers up to value nSample */
      clusterOtoC = malloc(nClusters*sizeof(int));
      if (!clusterOtoC)
	return cerror(ALLOC_ERR, 0);
      random_unique(0, clusterOtoC, nClusters, nSample);

      if (gotSample) {
	index = malloc(nSample*sizeof(int));
	if (!index) {
	  free(clusterOtoC);
	  return cerror(ALLOC_ERR, 0);
	}
	/* get nSample numbers up to value nVectors */
	random_unique(0, index, nSample, nVectors); /* get a sample */
	if (gotIndex) {
	  printf("CLUSTER - INDEX is ignored because SAMPLE is used\n");
	  useIndex = 0;
	}
	for (i = 0; i < nClusters; i++)
	  clusterOtoC[i] = index[clusterOtoC[i]];
      }

      for (i = 0; i < nClusters; i++) /* copy selected cluster centers */
	for (k = 0; k < nVectorDim; k++)
	  center[k + i*nVectorDim]
	    = (double) data[k + clusterOtoC[i]*nVectorDim];

      free(clusterOtoC);
      break;
    case ANA_ARRAY:			/* the array contains the initial */
      /* cluster centers.  The first dimension counts the dimensions */
      /* of each cluster center. */
      gotCenter = 1;		/* signal that the user specified */
				/* cluster centers */
      if (array_num_dims(ps[1]) < 2) /* only one cluster center */
	return anaerror("Finding just one cluster is silly!", ps[1]);
      if (array_dims(ps[1])[0] != nVectorDim) /* wrong # dimensions */
					      /* per data point */
	return anaerror("First dimension of DATA and CENTERS do not match",
		     ps[1]);
      nClusters = array_size(ps[1])/nVectorDim; /* # clusters */
      ana_replace(ps[1], ana_double(1, &ps[1]));
      center = (double *) array_data(ps[1]); /* data centers */

      if (gotSample) {
	if (nSample == 1) {
	  nSample = nVectors/10;
	  if (nSample < 10*nClusters)
	    nSample = 10*nClusters;
	}
	if (nSample < 1 || nSample > nVectors)
	  nSample = nVectors;
      } else
	nSample = nVectors;
      if (nSample == nVectors)
	gotSample = 0;

      if (gotSample) {
	index = malloc(nSample*sizeof(int));
	if (!index)
	  return cerror(ALLOC_ERR, 0);
	random_unique(0, index, nSample, nVectors); /* get a sample */
	if (gotIndex) {
	  printf("CLUSTER - INDEX is ignored because SAMPLE is used\n");
	  useIndex = 0;
	}
      }
      break;
    default:
      return cerror(ILL_CLASS, ps[1]);
  }

  if (gotIndex && useIndex) {	/* INDEX */
    /* if INDEX has the same number of elements as there are data points, */
    /* and if it has integer type, and if all elements are between 0 and */
    /* nClusters - 1, then we assume it to hold cluster numbers to update. */
    useIndex = 0;
    if (ps[2] >= NAMED_END)	/* INDEX is not a named variable */
      return anaerror("Argument must be a named variable", ps[2]);
    if (!gotSample		/* no SAMPLE */
	&& symbol_class(ps[2]) == ANA_ARRAY /* INDEX is an array */
	&& array_size(ps[2]) == nVectors /* an index for each data point */
	&& (int) array_type(ps[2]) <= ANA_LONG) { /* integer data type */
      indexType = array_type(ps[2]);	/* index data type */
      useIndex = 1;		/* default: useful indices */
      clusterNumber.b = (byte *) array_data(ps[2]);
      switch (indexType) {
	case ANA_BYTE:
	  for (i = 0; i < nVectors; i++)
	    if ((int) clusterNumber.b[i] >= nClusters) { /* index too large */
	      printf("CLUSTER - illegal index #%1d (%1d), ignore all\n",
		     i, clusterNumber.b[i]);
	      useIndex = 0;
	      break;
	    }
	  break;
	case ANA_WORD:
	  for (i = 0; i < nClusters; i++)
	    if (clusterNumber.w[i] < 0 /* index too small */
		|| clusterNumber.w[i] >= nClusters) { /* index too large */
	      printf("CLUSTER - illegal index #%1d (%1d), ignore all\n",
		     i, clusterNumber.w[i]);
	      useIndex = 0;
	      break;
	    }
	  break;
	case ANA_LONG:
	  for (i = 0; i < nClusters; i++)
	    if (clusterNumber.l[i] < 0 /* index too small */
		|| clusterNumber.l[i] >= nClusters) { /* index too large */
	      printf("CLUSTER - illegal index #%1d (%1d), ignore all\n",
		     i, clusterNumber.l[i]);
	      useIndex = 0;
	      break;
	    }
	  break;
      }
    } /* end of if (symbol_class(ps[2]) == ANA_ARRAY && ...) */
  }
  if (!useIndex) {	/* no sufficient memory yet for cluster numbers */
    if (nClusters - 1 <= UCHAR_MAX)	/* ANA_BYTE will do it */
      indexType = ANA_BYTE;
    else if (nClusters - 1 <= SHRT_MAX) /* ANA_WORD to store biggest index */
      indexType = ANA_WORD;
    else			/* need ANA_LONG */
      indexType = ANA_LONG;
    if (gotIndex && !gotSample)	{ /* have a variable, redefine */
      redef_array(ps[2], indexType , nDataDims - 1, dataDims + 1);
      clusterNumber.b = (byte *) array_data(ps[2]);
    } else {			/* no variable, allocate */
      clusterNumber.b = malloc(nSample*ana_type_size[indexType]*sizeof(byte));
      if (!clusterNumber.b)
	return cerror(ALLOC_ERR, 0);
    }
  }

  iterate = internalMode & 10;	/* /ITERATE */
  update = internalMode & 11;	/* /UPDATE */
  vocal = internalMode & 4;	/* /VOCAL */
  quick = internalMode & 8;	/* /QUICK */
  record = internalMode & 16;	/* /RECORD */
  ordered = internalMode & 32;	/* /ORDERED */

  if (record) {
    file = fopen("cluster.out", "w");
    if (!file)
      return anaerror("Could not open file cluster.out for writing", 0); }
  

  if (useIndex && !gotPhantom)
    phantom = 0;

  /* 2. Determine direction of greatest variation (DGV) */
  /* use an algorithm of my own devising; basically a 2-cluster analysis */
  dgv = malloc(nVectorDim*sizeof(double));
  if (!dgv)
    return cerror(ALLOC_ERR, 0);
  center2 = malloc(nVectorDim*2*sizeof(double));
  if (!center2)
    return cerror(ALLOC_ERR, 0);
  /* the first and last cluster centers form the initial centers of */
  /* the two clusters.  Take first and last because those are (close to) */
  /* furthest apart of all cluster centers if CENTERS is a result */
  /* from an earlier call to CLUSTER. */
  memcpy(center2, center, size); /* copy first cluster center */
  memcpy(center2 + nVectorDim,	/* copy last cluster center */
	 center + nVectorDim*(nClusters - 1), size);
  group1 = center2;		/* centroid of cluster 1 */
  group2 = center2 + nVectorDim; /* centroid of cluster 2 */
  for (i = 0; i < nVectorDim; i++) /* get mean position of the centroids */
    dgv[i] = 0.5*(group1[i] + group2[i]);
  n1 = n2 = 1;  n = 2;		/* number of points in groups 1, 2, and both */
  for (i = 2; i < nClusters; i++) { /* all remaining cluster centers */
    f = 0.0;			/* initialize projection */
    fp = center + i*nVectorDim;	/* current data point (i.e. cluster center) */
    for (j = 0; j < nVectorDim; j++) /* calculate projection on line */
				     /* connecting both centroids */
      f += (fp[j] - dgv[j])*(group2[j] - group1[j]);
    if (f > 0) {		/* enter into group 2 */
      for (j = 0; j < nVectorDim; j++) /* update group 2 centroid */
	group2[j] = (n2*group2[j] + fp[j])/(n2 + 1);
      n2++;
    } else if (f < 0) {		/* enter into group 1 */
      for (j = 0; j < nVectorDim; j++) /* update group 1 centroid */
	group1[j] = (n1*group1[j] + fp[j])/(n1 + 1);
      n1++;
    } else {			/* put half in each group */
      for (j = 0; j < nVectorDim; j++) {
	group1[j] = (2*n1*group1[j] + fp[j])/(2*n1 + 1);
	group2[j] = (2*n2*group2[j] + fp[j])/(2*n2 + 1);
      }
      n1 += 0.5;
      n2 += 0.5;
    }
    n = n1 + n2;		/* update total number */
    for (j = 0; j < nVectorDim; j++) /* update centroid average */
      dgv[j] = (n1*group1[j] + n2*group2[j])/n;
  }
  /* now group1 contains the average position of points in group1, and */
  /* group2 the average position of points in group2.  The difference */
  /* between these two positions is a measure for the direction of */
  /* greatest variation (DGV). */
  t = 0.0;			/* initialize length of DGV vector */
  for (i = 0; i < nVectorDim; i++) { /* calculate unnormalized DGV */
    dgv[i] = group2[i] - group1[i];
    t += dgv[i]*dgv[i];		/* accumulate length of DGV */
  }
  t = sqrt(t);			/* length of unnormalized DGV */
  d = s = 0.0;
  /* normalize the DGV, and find sign of its largest compoennt */
  if (t > 0)
    for (i = 0; i < nVectorDim; i++) {
      dgv[i] /= t;		/* normalize to length 1 */
      if (fabs(dgv[i]) > d) {	/* this component is biggest so far */
	s = dgv[i];		/* save biggest component */
	d = fabs(s); 		/* and its magnitude */
      }
    }
  /* force the sign of the largest component to be positive, otherwise */
  /* INDEX may appear to "change sign" from CLUSTER call to CLUSTER call. */
  /* Change sign of all components if the largest component is negative */
  if (s < 0)			/* largest component is negative */
    for (i = 0; i < nVectorDim; i++)
      dgv[i] = -dgv[i];		/* change sign of all components */
  free(center2);

  /* 3. Calculate projections of initial cluster centers on DGV (PDGV) */
  /* it is advantageous to have a sentinel on each end of the PDGV array
     with extreme values.  These extreme values must be small enough
     that their squares can be represented by a variable of type double. */
  pdgv = malloc((nClusters + 2)*sizeof(double));
  if (!pdgv)
    return cerror(ALLOC_ERR, 0);
  pdgv[0] = -0.9*sqrt(DBL_MAX);	/* sentinel */
  pdgv[nClusters + 1] = 0.9*sqrt(DBL_MAX); /* sentinel */
  pdgv++;			/* now pdgv[i] points at the entry for
				   cluster i. */
  for (i = 0; i < nClusters; i++) { /* all cluster centers */
    pdgv[i] = 0.0;		/* initial value */
    for (j = 0; j < nVectorDim; j++) /* project */
      pdgv[i] += dgv[j]*center[i*nVectorDim + j];
  }

  nChanged = 1;			/* non-zero to get iteration going */
  /* 4. Order cluster centers according to their PDGV */
  findex = malloc(nClusters*sizeof(double *));
  center2 = malloc(nClusters*nVectorDim*sizeof(double));
  if (!findex || !center2)
    return cerror(ALLOC_ERR, 0);
  clusterSize = NULL;
  while (nChanged) {
    for (i = 0; i < nClusters; i++)
      findex[i] = pdgv + i;	/* current order */
    qsort(findex, nClusters, sizeof(double *), fptrCompare); /* sort */
    for (i = 0; i < nClusters; i++) /* reorder PDGVs */
      center2[i] = *findex[i];
    memcpy(pdgv, center2, nClusters*sizeof(double));
    for (i = 0; i < nClusters; i++) /* reorder cluster centers */
      memcpy(center2 + i*nVectorDim, center + (findex[i] - pdgv)*nVectorDim,
	     size);
    memcpy(center, center2, size*nClusters);
    /* if we took a random sample of the data points for initial cluster */
    /* centers, then we may have multiple copies of one or more cluster */
    /* centers if there are multiple copies of one or more data points. */
    /* We need to check for this, because we need unique cluster centers. */
    nChanged = 0;
    if (!gotCenter) {		/* we took a random sample */
      for (i = 1; i < nClusters; i++)
	if (pdgv[i] == pdgv[i - 1]) { /* we may have a double */
	  if (!nChanged) {	/* get some random indices */
	    clusterSize = malloc(10*sizeof(int));
	    if (!clusterSize)
	      return cerror(ALLOC_ERR, 0);
	  }
	  if (nChanged % 10 == 0) /* need 10 new random indices */
	    randomu(0, clusterSize, 10, nVectors);
	  for (k = 0; k < nVectorDim; k++)
	    center[k + i*nVectorDim]
	      = (double) data[k + clusterSize[nChanged % 10]*nVectorDim];
	  pdgv[i] = 0.0;
	  for (j = 0; j < nVectorDim; j++)
	    pdgv[i] += center[j + i*nVectorDim]*dgv[j];
	  nChanged++; }
      if (nChanged && vocal)
	printf("CLUSTER - got %1d new centers because of double data points\n",
	       nChanged);
    }
  } /* end while (nChanged) */
  if (clusterSize)
    free(clusterSize);
  free(center2);

  /* if the user specified cluster centers (i.e. CENTERS is an array) */
  /* then we need to remember the original order, so we can go from the */
  /* old to the new order and back, and can restore the original order */
  /* on exit */
  clusterCtoO = malloc(nClusters*sizeof(int));
  clusterOtoC = malloc(nClusters*sizeof(int));
  if (!clusterCtoO || !clusterOtoC)
    return cerror(ALLOC_ERR, 0);
  if (gotCenter)		/* save original order */
    for (i = 0; i < nClusters; i++) {
      clusterOtoC[findex[i] - pdgv] = i;
      clusterCtoO[i] = findex[i] - pdgv;
    }
  else				/* no original order to save, so save */
				/* current one */
    for (i = 0; i < nClusters; i++)
      clusterCtoO[i] = clusterOtoC[i] = i;
  free(findex);

  if (gotSize) {		/* SIZE was specified by user */
    redef_array(ps[3], ANA_LONG, 1, &nClusters); /* get in shape */
    clusterSize = (int *) array_data(ps[3]);
  } else {				/* no SIZE specified */
    clusterSize = malloc(nClusters*sizeof(int)); /* so allocate some space */
    if (!clusterSize)
      return cerror(ALLOC_ERR, 0);
  }
  for (i = 0; i < nClusters; i++)
    clusterSize[i] = phantom;	/* add phantom members, if any */
  if (useIndex			/* usable cluster numbers */
      && (update || gotSize)	/* going to update cluster centers */
      && !gotPhantom) {		/* user did not ask for phantom clusters */
    switch (indexType) {
      case ANA_BYTE:
	for (i = 0; i < nSample; i++) {
	  j = clusterOtoC[clusterNumber.b[i]];
	  clusterSize[j]++;
	}
	break;
      case ANA_WORD:
	for (i = 0; i < nSample; i++) {
	  j = clusterOtoC[clusterNumber.w[i]];
	  clusterSize[j]++;
	}
	break;
      case ANA_LONG:
	for (i = 0; i < nSample; i++) {
	  j = clusterOtoC[clusterNumber.l[i]];
	  clusterSize[j]++;
	}
	break;
    }
    recluster = 1;
  } else 
    recluster = 0;

  if (phantom) {		/* we added phantom members, so now */
    /* we must remember their positions so we can remove them afterwards */
    firstCenter = malloc(nClusters*nVectorDim*sizeof(double));
    if (!firstCenter)
      return cerror(ALLOC_ERR, 0);
    for (i = 0; i < nClusters; i++)
      memcpy(firstCenter + i*nVectorDim,
	     center + clusterOtoC[i]*nVectorDim, size);
  }

  /* initialize the centroids */
  if (update)			/* we're updating, so centroid = center */
    centroid = center;
  else {			/* not updating, so separate centroids */
    centroid = malloc(nClusters*nVectorDim*sizeof(double));
    if (!centroid)
      return cerror(ALLOC_ERR, 0);
    memcpy(centroid, center, nClusters*size);
  }

  scrap = malloc(nVectorDim*sizeof(double)); /* scrap space */

  /* we want to count the number of changed points and keep track of */
  /* which clusters change */
  changed = malloc(nClusters*sizeof(char));
  changedOld = malloc(nClusters*sizeof(char));
  if (!scrap || !changed || !changedOld)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < nClusters; i++)
    changedOld[i] = 1;		/* so all clusters are treated in */
				/* the first iteration */

  if (rms) {
    redef_array(rms, ANA_DOUBLE, 1, &nClusters);
    rmsptr = array_data(rms);
  }

  nChanged = 1;			/* number of data points that changed */
	/* cluster during the last iteration - must be nonzero here to */
	/* get the first iteration going */

  k = k0 = nClusters/2;		/* start searching for the closest cluster */
				/* center in the middle */

  nIter = 0;			/* iteration number */
  while (nChanged) {		/* not stable yet */
    nIter++;
    for (i = 0; i < nClusters; i++)
      changed[i] = 0;		/* no clusters changed yet */
    nChanged = 0;		/* no data points changed yet */
    if (rms)
      zerobytes(rmsptr, nClusters*ana_type_size[ANA_DOUBLE]);
    for (i = 0; i < nSample; i++) { /* treat all selected data points */
      dataIndex = gotSample? index[i]: i;
      dataPoint = data + dataIndex*nVectorDim; /* current data point */
      if (useIndex)		/* get old cluster number */
	switch (indexType) {
	  case ANA_BYTE:
	    curO = *clusterNumber.b;
	    break;
	  case ANA_WORD:
	    curO = *clusterNumber.w;
	    break;
	  case ANA_LONG:
	    curO = *clusterNumber.l;
	    break;
	}
      if (!useIndex || changedOld[curO] || !quick) {
	/* 6. calculate the PDGV of the current data point */
	s = 0.0;
	for (j = 0; j < nVectorDim; j++)
	  s += dgv[j]*((double) dataPoint[j]); /* s = PDGV */
	
	/* 7. Find the closest cluster center according to PDGVs */
	/* because of the sentinels at both ends of the PDGV array we
	   don't need to worry about venturing beyond its ends. */
	while (s >= pdgv[k + 1]) /* current PDGV >= cluster PDGV */
	  k++;			/* next cluster */
	while (s < pdgv[k])	/* current PDGV < cluster PDGV */
	  k--;			/* previous cluster */
	/* now s is between clusters k and k+1 in PDGV */
	if (ABS(pdgv[k + 1] - s) < ABS(pdgv[k] - s)) /* cluster k+1 */
				/* is closer in PDGV to data point than k */
	  k++;			/* increment */
	/* now k is the index of the closest (in PDGV) cluster center */
	
	/* 8. Calculate total distance to that cluster center */
	dMin2 = 0.0;
	for (l = 0; l < nVectorDim; l++) {
	  t = ((double) dataPoint[l] - center[l + k*nVectorDim]);
	  dMin2 += t*t;
	}

	nDistCal++;
	
	if (nIter == 1 && ordered && k != k0) { /* try the previous point */
	  d = 0.0;
	  for (l = 0; l < nVectorDim; l++) {
	    t = ((double) dataPoint[l] - center[l + k0*nVectorDim]);
	    d += t*t;
	  }
	  if (d < dMin2) {
	    k = k0;
	    dMin2 = d;
	  }
	}

	if (useIndex && k != k0) { /* try the previous iteration's result */
	  k0 = clusterOtoC[curO];
	  d = 0.0;
	  for (l = 0; l < nVectorDim; l++) {
	    t = ((double) dataPoint[l] - center[l + k0*nVectorDim]);
	    d += t*t;
	  }
	  if (d < dMin2) {
	    k = k0;
	    dMin2 = d;
	  }
	}

	/* 9. Determine which (if any) other centers need be considered, */
	/* using PDGV criterion: if dPDGV > dMin then the cluster center */
	/* is certainly further away.  first, go to smaller PDGV */
	/* Because of the sentinels at the ends of pdgv we don't need to
	   worry about inadvertently venturing beyond its end. */
	/* Calculating square roots takes *much* more time than calculating
	   a square, so we work with squared distances */
	k0 = k;			/* remember starting position */
	for (l = k - 1; (pdgv[l] - s)*(pdgv[l] - s) <= dMin2; l--) {
	  /* all centers that have pdgv[l] >= s - dMin may be closer to */
	  /* the data point than center k is.  Check them, starting at */
	  /* k - 1, and keep updating the admissibility.  (dMin2 = dMin^2)*/
	  d = 0.0;		/* initialize distance */
	  for (m = 0; m < nVectorDim; m++) {
	    t = ((double) dataPoint[m] - center[m + l*nVectorDim]);
	    d += t*t;
	  }

	  nDistCal++;
	
	  if (d < dMin2) {	/* this center is closer than closest up */
				/* till now, so update */
	    dMin2 = d;		/* new minimum distance */
	    k = l;		/* new closest cluster */
	  }
	} /* end of for (l = k - 1; pdgv[l] >= s - dMin; l--) */
	kBest = k;		/* closest cluster below initial one */
	/* Now, go to larger PDGV */
	k = k0;			/* start at initial cluster again */
	for (l = k + 1; (pdgv[l] - s)*(pdgv[l] - s) <= dMin2; l++) {
	  /* all centers with pdgv[l] <= s + dMin may be closer to */
	  /* the data point than center k is.  Check them, starting at */
	  /* k + 1, and keep updating the admissibility. */
	  d = 0.0;		/* initialize distance */
	  for (m = 0; m < nVectorDim; m++) {
	    t = ((double) dataPoint[m] - center[m + l*nVectorDim]);
	    d += t*t;
	  }

	  nDistCal++;
	
	  if (d < dMin2) {	/* this center is closer than closest up */
				/* till now, so update */
	    dMin2 = d;		/* new minimum distance */
	    k = l;		/* new closest cluster */
	  }
	} /* end of for (l = k + 1; pdgv[l] <= s + dMin; l++) */
	if (k == k0)		/* pointer to best cluster is unchanged, */
	  k = kBest;		/* so the old one is the best */
	/* k points at the cluster center closest to the current data point */
	newO = clusterCtoO[k];	/* cluster number in old ordering */
	if (rms)
	  rmsptr[newO] += dMin2; /* add squared distance */

	/* do we need to update any centroid? */
	if (useIndex) {
	  if (curO != newO) { /* this data point changed cluster */
	    changed[curO] = 1;	/* old cluster changed */
	    changedOld[curO] = 1; /* start treating remaining data points */
		/* in this cluster in the current iteration, too */
	    changed[newO] = 1; /* new cluster changed */
	    nChanged++;		/* one more reclustered point */
	    curChanged = 1;
	  } else
	    curChanged = 0;
	} else {
	  nChanged++;
	  curChanged = 1;
	}

	if (curChanged && recluster) { /* remove data point from old cluster */
	  j = clusterOtoC[curO]; /* old cluster in new ordering */
	  l = clusterSize[j]--;
	  for (m = 0; m < nVectorDim; m++) /* update old centroid */
	    centroid[m + j*nVectorDim]
	      = (centroid[m + j*nVectorDim]*l - (double) dataPoint[m])/(l - 1);
	  if (update) {
	    s = 0.0;		/* recalculate PDGVs */
	    for (m = 0; m < nVectorDim; m++)
	      s += dgv[m]*center[m + j*nVectorDim];
	    /* do we need to reorder? */
	    /* where does s fit in current PDGVs? */
	    k2 = j;
	    while (k2		/* not yet at first cluster */
		   && s < pdgv[k2 - 1]) /* need to go lower */
	      k2--;
	    if (k2 < j) {	/* reorder below j */
	      n = sizeof(int)*(j - k2);
	      memmove(pdgv + k2 + 1, pdgv + k2, sizeof(double)*(j - k2));
	      pdgv[k2] = s;
	      k0 = clusterSize[j];
	      memmove(clusterSize + k2 + 1, clusterSize + k2, n);
	      clusterSize[k2] = k0;
	      k0 = clusterCtoO[j];
	      for (l = j; l > k2; l--) {
		clusterCtoO[l] = clusterCtoO[l - 1];
		clusterOtoC[clusterCtoO[l]] = l;
	      }
	      clusterCtoO[k2] = k0;
	      clusterOtoC[clusterCtoO[k2]] = k2;
	      memcpy(scrap, center + j*nVectorDim, size);
	      memmove(center + (k2 + 1)*nVectorDim, center + k2*nVectorDim,
		      size*(j - k2));
	      memcpy(center + k2*nVectorDim, scrap, size);
	      if (k >= k2 && k < j) /* must now update found cluster number */
		k++;
	      else if (k == j)
		k = k2;
	    } else {		/* no reordering below current j, but */
				/* perhaps above it? */
	      while (k2 < nClusters - 1 /* not yet at last cluster */
		     && s > pdgv[k2 + 1]) /* need to go higher */
		k2++;
	      if (k2 > j) {	/* reorder above j */
		n = sizeof(int)*(k2 - j);
		memmove(pdgv + j, pdgv + j + 1, sizeof(double)*(k2 - j));
		pdgv[k2] = s;
		k0 = clusterSize[j];
		memmove(clusterSize + j, clusterSize + j + 1, n);
		clusterSize[k2] = k0;
		k0 = clusterCtoO[j];
		for (l = j; l < k2; l++) {
		  clusterCtoO[l] = clusterCtoO[l + 1];
		  clusterOtoC[clusterCtoO[l]] = l;
		}
		clusterCtoO[k2] = k0;
		clusterOtoC[clusterCtoO[k2]] = k2;
		memcpy(scrap, center + j*nVectorDim, size);
		memmove(center + j*nVectorDim, center + (j + 1)*nVectorDim,
			size*(k2 - j));
		memcpy(center + k2*nVectorDim, scrap, size);
		if (k <= k2 && k > j) /* need to update found cluster number */
		  k--;
		else if (k == j)
		  k = k2; }	/* end if (k2 > j) */
	      else		/* if (k2 == j) */
		pdgv[j] = s;	/* no reordering, but still update */
	    } /* end if (k2 < j) else */
	  } /* end if (update) */
	} /* end if (recluster) */

	/* now add point to new cluster */
	if (curChanged || !useIndex || gotPhantom) {
	  l = clusterSize[k]++;	/* add point to receiving cluster */
	  for (m = 0; m < nVectorDim; m++) /* update centroid */
	    centroid[m + k*nVectorDim]
	      = (centroid[m + k*nVectorDim]*l + (double) dataPoint[m])/(l + 1);
	  if (update) {
	    s = 0.0;		/* recalculate PDGV */
	    for (m = 0; m < nVectorDim; m++)
	      s += dgv[m]*centroid[m + k*nVectorDim];
	    /* do we need to reorder the centers? */
	    /* where does s fit in PDGVs? */
	    k2 = k;
	    while (k2		/* not yet at first cluster */
		   && s < pdgv[k2 - 1]) /* need to go lower */
	      k2--;
	    if (k2 < k) {	/* reorder below k */
	      n = sizeof(int)*(k - k2);
	      memmove(pdgv + k2 + 1, pdgv + k2, sizeof(double)*(k - k2));
	      pdgv[k2] = s;
	      k0 = clusterSize[k];
	      memmove(clusterSize + k2 + 1, clusterSize + k2, n);
	      clusterSize[k2] = k0;
	      k0 = clusterCtoO[k];
	      for (l = k; l > k2; l--) {
		clusterCtoO[l] = clusterCtoO[l - 1];
		clusterOtoC[clusterCtoO[l]] = l;
	      }
	      clusterCtoO[k2] = k0;
	      clusterOtoC[clusterCtoO[k2]] = k2;
	      memcpy(scrap, centroid + k*nVectorDim, size);
	      memmove(centroid + (k2 + 1)*nVectorDim, centroid + k2*nVectorDim,
		      size*(k - k2));
	      memcpy(centroid + k2*nVectorDim, scrap, size);
	    } else {		/* if (k2 >= k) */
	      while (k2 < nClusters - 1 /* not yet at last cluster */
		     && s > pdgv[k2 + 1]) /* need to go higher */
		k2++;
	      if (k2 > k) {	/* reorder above k */
		n = sizeof(int)*(k2 - k);
		memmove(pdgv + k, pdgv + k + 1, sizeof(double)*(k2 - k));
		pdgv[k2] = s;
		k0 = clusterSize[k];
		memmove(clusterSize + k, clusterSize + k + 1, n);
		clusterSize[k2] = k0;
		k0 = clusterCtoO[k];
		for (l = k; l < k2; l++) {
		  clusterCtoO[l] = clusterCtoO[l + 1];
		  clusterOtoC[clusterCtoO[l]] = l;
		}
		clusterCtoO[k2] = k0;
		clusterOtoC[clusterCtoO[k2]] = k2;
		memcpy(scrap, centroid + k*nVectorDim, size);
		memmove(centroid + k*nVectorDim, centroid + (k + 1)*nVectorDim,
			size*(k2 - k));
		memcpy(centroid + k2*nVectorDim, scrap, size);
	      } else
		pdgv[k] = s;	/* if (k2 <= k) */
	    } /* end if (k2 < j) else */
	  } /* end if (update) */
	} /* end if (!recluster && (!useIndex || gotPhantom || !update)) */
      }	/* end if (!useIndex || changedOld[curO] || !quick) */
      else		/* if (useIndex && !changedOld[curO] && quick) */
	newO = curO;
      
      switch (indexType) {	/* save cluster number */
	case ANA_BYTE:
	  *clusterNumber.b++ = newO;
	  break;
	case ANA_WORD:
	  *clusterNumber.w++ = newO;
	  break;
	case ANA_LONG:
	  *clusterNumber.l++ = newO;
	  break;
      }
      
      k0 = clusterOtoC[newO];	/* initial cluster number for next one */
      
    } /* end for (i = 0; i < nSample; i++) */

    if (record && iterate) {
      for (i = 0; i < nClusters; i++)
	fwrite(centroid + clusterOtoC[i], sizeof(double), nVectorDim, file);
      for (i = 0; i < nClusters; i++)
	fwrite(clusterSize + clusterOtoC[i], sizeof(int), 1, file);
    }
    
    /* remove phantom members, if any */
    if (!gotPhantom && phantom) {
      for (i = 0; i < nClusters; i++) {
	for (j = 0; j < nVectorDim; j++)
	  centroid[j + i*nVectorDim] =
	    (clusterSize[i]*centroid[j + i*nVectorDim]
	     - firstCenter[j + clusterCtoO[i]*nVectorDim]*phantom)
	      /(clusterSize[i] - phantom);
	clusterSize[i] -= phantom;
      }
      free(firstCenter);
      phantom = 0;

      if (nChanged && iterate) {
	/* update pdgvs */
	k = 0;
	for (i = 0; i < nClusters; i++) { /* all cluster centers */
	  pdgv[i] = 0.0;	/* initial value */
	  for (j = 0; j < nVectorDim; j++) /* project */
	    pdgv[i] += dgv[j]*centroid[i*nVectorDim + j];
	  if (i && pdgv[i] < pdgv[i - 1])
	    k = 1; 		/* need to reorder */
	}
	if (k) {		/* reorder */
	  findex = malloc(nClusters*sizeof(double *));
	  if (!findex)
	    return cerror(ALLOC_ERR, 0);
	  for (i = 0; i < nClusters; i++)
	    findex[i] = pdgv + i; /* current order */
	  qsort(findex, nClusters, sizeof(double *), fptrCompare); /* sort */
	  center2 = malloc(nClusters*nVectorDim*sizeof(double));
	  if (!center2)
	    return cerror(ALLOC_ERR, 0);
	  for (i = 0; i < nClusters; i++) /* reorder PDGVs */
	    center2[i] = *findex[i];
	  memcpy(pdgv, center2, nClusters*sizeof(double));
	  if (center != centroid) {
	    for (i = 0; i < nClusters; i++) /* reorder cluster centers */
	      memcpy(center2 + i*nVectorDim,
		     center + (findex[i] - pdgv)*nVectorDim, size);
	    memcpy(center, center2, size*nClusters);
	  }
	  for (i = 0; i < nClusters; i++) /* reorder cluster centroids */
	    memcpy(center2 + i*nVectorDim,
		   centroid + (findex[i] - pdgv)*nVectorDim, size);
	  memcpy(centroid, center2, size*nClusters);
	  free(center2);
	  iPtr = malloc(nClusters*sizeof(int));
	  if (!iPtr)
	    return cerror(ALLOC_ERR, 0);
	  for (i = 0; i < nClusters; i++) /* reorder cluster sizes */
	    iPtr[i] = clusterSize[findex[i] - pdgv];
	  memcpy(clusterSize, iPtr, nClusters*sizeof(int));
	  for (i = 0; i < nClusters; i++) { /* reorder CtoO and OtoC */
	    iPtr[i] = clusterCtoO[findex[i] - pdgv];
	    clusterOtoC[iPtr[i]] = i;
	  }
	  memcpy(clusterCtoO, iPtr, nClusters*sizeof(int));
	  free(iPtr);
	}
      }	/* end if (nChanged && iterate) */

    } /* end if (!gotPhantom && phantom) */

    allDistCal += nDistCal;

    if (vocal) {
      if (useIndex) {
	j = 0;
	for (i = 0; i < nClusters; i++)
	  if (changed[i])
	    j++;
      } else
	j = nClusters;
      printf("CLUSTER - cycle %1d, reclustered %1d points in %1d clusters\n",
	     nIter, nChanged, j);
      printf("distance calculations/element: %g (total %g)\n",
	     (float) nDistCal/nSample, (float) allDistCal/nSample);
    }
    
    nDistCal = 0;
    
				/* reinitialize for a next iteration */
    clusterNumber.b -= nSample*ana_type_size[indexType];
    if (iterate && nChanged) {
      if (useIndex)
	memcpy(changedOld, changed, nClusters);
      useIndex = 1;
      if (gotPhantom) {
	for (i = 0; i < nClusters; i++)
	  clusterSize[i] = phantom;
	memcpy(firstCenter, centroid, nClusters*size);
      } else if (update)
	recluster = 1;
    } else
      nChanged = 0;		/* force exit */

    if (nIter == maxit)		/* reached maximum number of iterations */
      nChanged = 0;
  } /* end of while (nChanged) */
  
  if (record && iterate)
    fclose(file);

  /* restore the centers to their original order */
  center2 = malloc(nClusters*nVectorDim*sizeof(double));
  if (!center2)
    return cerror(ALLOC_ERR, 0);
  if (center != centroid && update) { /* updating */
    memcpy(center2, centroid, nClusters*size);
    for (i = 0; i < nClusters; i++)
      memcpy(centroid + i*nVectorDim,
	     center2 + clusterOtoC[i]*nVectorDim, size);
    memcpy(center, centroid, nClusters*size);
  } else {
    memcpy(center2, center, nClusters*size);
    for (i = 0; i < nClusters; i++)
      memcpy(center + i*nVectorDim,
	     center2 + clusterOtoC[i]*nVectorDim, size);
  }
  free(center2);
  if (center != centroid)
    free(centroid);

  /* reorder the sizes */
  index = malloc(nClusters*sizeof(int));
  if (!index)
    return cerror(ALLOC_ERR, 0);
  memcpy(index, clusterSize, nClusters*sizeof(int));
  for (i = 0; i < nClusters; i++)
    clusterSize[i] = index[clusterOtoC[i]];
  free(index);

  /* remove phantom members, if any */
  if (phantom) {
    for (i = 0; i < nClusters; i++) {
      for (j = 0; j < nVectorDim; j++)
	center[j + i*nVectorDim] =
	  (clusterSize[i]*center[j + i*nVectorDim]
	   - firstCenter[j + i*nVectorDim]*phantom)
	  /(clusterSize[i] - phantom);
      clusterSize[i] -= phantom;
    }
    free(firstCenter);
  }

  if (rms)
    for (i = 0; i < nClusters; i++)
      rmsptr[i] = sqrt(rmsptr[i]/clusterSize[i]);

  /* get rid of dangling allocated memory */

  if (!gotSize)
    free(clusterSize);
  free(dgv);
  free(pdgv - 1);		/* minus one to account for the first
				   sentinel */
  free(clusterCtoO);
  free(clusterOtoC);
  free(changed);
  free(changedOld);
  free(scrap);
  if (!gotIndex || gotSample)
    free(clusterNumber.b);
  return ANA_OK;
}
/*----------------------------------------------------------------*/
