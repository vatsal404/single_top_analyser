using namespace ROOT::VecOps;
using floats =  ROOT::VecOps::RVec<float>;
using doubles =  ROOT::VecOps::RVec<double>;

floats sort_discriminant( floats discr, floats obj ){
	auto sorted_discr = Reverse(Argsort(discr));// max to min
		floats out;
		for (auto idx : sorted_discr){
			out.emplace_back(obj[idx]);
		}
		return out;
}

void PrintVector(floats myvector)
{
	for (size_t i = 0; i < myvector.size(); i++){
		std::cout<<myvector[i]<<"\n";
	}

}

 void ArgsortExample()
{
	//0 ,1 ,2 
 	RVec<float> v1 {2., 3., 1.,8.};
 	RVec<float> v2 {5., 7., 2.,10.};
   //auto sortIndices = Argsort(v);

    RVec<float> v3 =sort_discriminant(v1,v1);
	RVec<float> v4 =sort_discriminant(v1,v2);

     PrintVector(v3);
	 //PrintVector(v4);
} 


