#ifndef APRIL_DBSCAN_H
#define APRIL_DBSCAN_H

// this an example to use mlpack and dlib
// For dlib, we have to define __COMPILE_DLIB__ from cmake configuration.

namespace april_content
{

class APRILDBSCAN
{
public:

	APRILDBSCAN();

#if __COMPILE_DLIB__
	void DoClustering();
	void DoGraph();
#endif

private:

};

}

#endif
