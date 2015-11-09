//! PRNG service. Implemented by the core, do not reimplement.  Use g_create() helper function to instantiate.
class NOVTABLE genrand_service : public service_base
{
public:
	//! Seeds the PRNG with specified value.
	virtual void seed(unsigned val) = 0;
	//! Returns random value N, where 0 <= N < range.
	virtual unsigned genrand(unsigned range)=0;

	static service_ptr_t<genrand_service> g_create() {return standard_api_create_t<genrand_service>();}

	void generate_random_order(t_size * out, t_size count) {
		t_size n;
		for(n=0;n<count;n++) out[n]=n;
		for(n=0;n<count;n++) pfc::swap_t(out[n],out[genrand(count)]);
	}

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(genrand_service);
};
