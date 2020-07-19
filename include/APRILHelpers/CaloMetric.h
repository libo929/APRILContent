#ifndef CALOMETRIC_H
#define CALOMETRIC_H

namespace april_content
{

class CaloMetric
{
   public:

    // default constructor does nothing, but is required to satisfy the Metric policy.
    CaloMetric() { }

	// metric for calorimter hit (x, y, z, layer)
    template<typename VecTypeA, typename VecTypeB>
    static typename VecTypeA::elem_type Evaluate(const VecTypeA& a, const VecTypeB& b)
    {
		//assert(a.size() == 4 && b.size() == 4);

		return sqrt( (a[0]-b[0]) * (a[0]-b[0]) + (a[1]-b[1]) * (a[1]-b[1]) + (a[2]-b[2]) * (a[2]-b[2]) ) 
			* ( fabs(a[3]-b[3]) < 1.e-5 ? 1.e5 : 1);
    }
};

}

#endif
