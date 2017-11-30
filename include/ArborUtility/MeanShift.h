#ifndef MEANSHIFT_H
#define MEANSHIFT_H

#include <vector>

typedef std::vector<double> Point;

struct Cluster 
{
    Point mode;
    std::vector<Point> original_points;
    std::vector<Point> shifted_points;
};

class MeanShift 
{
public:
    MeanShift(double kernelBandwidth = 0.1, double clusterEpsilon = 0.5, double shiftEpsilon = 0.00001);

    std::vector<Cluster> cluster(const std::vector<Point>&);

private:
    void shift_point(const Point&, const std::vector<Point>&, double, Point&);
    std::vector<Point> meanshift(const std::vector<Point>& points, double kernel_bandwidth, double shiftEpsilon);
    std::vector<Cluster> cluster(const std::vector<Point>&, const std::vector<Point>&, double clusterEpsilon);

    double m_kernelBandwidth;
	double m_clusterEpsilon;
	double m_shiftEpsilon;
};

#endif // MEANSHIFT_H
