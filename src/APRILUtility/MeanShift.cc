#include <stdio.h>
#include <math.h>

#include "ArborUtility/MeanShift.h"

using namespace std;

double euclidean_distance(const vector<double> &point_a, const vector<double> &point_b){
    double total = 0;
    for(int i=0; i<point_a.size(); i++){
        const double temp = (point_a[i] - point_b[i]);
        total += temp*temp;
    }
    return sqrt(total);
}

double euclidean_distance_sqr(const vector<double> &point_a, const vector<double> &point_b){
    double total = 0;
    for(int i=0; i<point_a.size(); i++){
        const double temp = (point_a[i] - point_b[i]);
        total += temp*temp;
    }
    return (total);
}

double gaussian_kernel(double distance, double kernel_bandwidth){
    double temp =  exp(-1.0/2.0 * (distance*distance) / (kernel_bandwidth*kernel_bandwidth));
    return temp;
}


MeanShift::MeanShift(double kernelBandwidth, double clusterEpsilon, double shiftEpsilon)
	      : m_kernelBandwidth(kernelBandwidth), m_clusterEpsilon(clusterEpsilon), m_shiftEpsilon(shiftEpsilon)
{
}

void MeanShift::shift_point(const MSPoint &point, const std::vector<MSPoint> &points, double kernel_bandwidth, MSPoint &shifted_point) 
{
    shifted_point.resize(point.size());
    
	for(int dim = 0; dim<shifted_point.size(); dim++)
	{
        shifted_point[dim] = 0;
    }

    double total_weight = 0;

    for(int i=0; i<points.size(); i++)
	{
        const MSPoint& temp_point = points[i];
        double distance = euclidean_distance(point, temp_point);
        double weight = gaussian_kernel(distance, kernel_bandwidth);

        for(int j=0; j<shifted_point.size(); j++)
		{
            shifted_point[j] += temp_point[j] * weight;
        }

        total_weight += weight;
    }

    const double total_weight_inv = 1.0/total_weight;

    for(int i=0; i<shifted_point.size(); i++)
	{
        shifted_point[i] *= total_weight_inv;
    }

	shifted_point.m_caloHit = point.m_caloHit;
}

std::vector<MSPoint> MeanShift::meanshift(const std::vector<MSPoint> &points, double kernel_bandwidth, double shiftEpsilon)
{
    double EPSILON_SQR = shiftEpsilon * shiftEpsilon;

    vector<bool> stop_moving(points.size(), false);
    vector<MSPoint> shifted_points = points;
    double max_shift_distance;

    MSPoint point_new;

    do {
        max_shift_distance = 0;

        for(int i=0; i<points.size(); i++)
		{
            if (!stop_moving[i]) 
			{
                shift_point(shifted_points[i], points, kernel_bandwidth, point_new);
                double shift_distance_sqr = euclidean_distance_sqr(point_new, shifted_points[i]);

                if(shift_distance_sqr > max_shift_distance)
				{
                    max_shift_distance = shift_distance_sqr;
                }

                if(shift_distance_sqr <= EPSILON_SQR) 
				{
                    stop_moving[i] = true;
                }

                shifted_points[i] = point_new;
            }
        }
        //printf("max_shift_distance: %f\n", sqrt(max_shift_distance));
    } while (max_shift_distance > EPSILON_SQR);
    return shifted_points;
}

vector<MSCluster> MeanShift::cluster(const std::vector<MSPoint> &points, const std::vector<MSPoint> &shifted_points, double clusterEpsilon)
{
    vector<MSCluster> clusters;

    for (int i = 0; i < shifted_points.size(); i++) 
	{
        int c = 0;

        for (; c < clusters.size(); c++) 
		{
            if (euclidean_distance(shifted_points[i], clusters[c].mode) <= clusterEpsilon) break;
        }

        if (c == clusters.size()) {
            MSCluster clus;
            clus.mode = shifted_points[i];
            clusters.push_back(clus);
        }

        clusters[c].original_points.push_back(points[i]);
        clusters[c].shifted_points.push_back(shifted_points[i]);
    }

    return clusters;
}

vector<MSCluster> MeanShift::cluster(const std::vector<MSPoint> &points)
{
    vector<MSPoint> shifted_points = meanshift(points, m_kernelBandwidth, m_shiftEpsilon);

    return cluster(points, shifted_points, m_clusterEpsilon);
}
