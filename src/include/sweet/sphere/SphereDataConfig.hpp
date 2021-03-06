/*
 * SPHSetup.hpp
 *
 *  Created on: 12 Aug 2016
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk> Schreiber <M.Schreiber@exeter.ac.uk>
 */

#ifndef SPHSETUP_HPP_
#define SPHSETUP_HPP_


#include <libmath/shtns_inc.hpp>
#include <fftw3.h>
#include <iostream>
#include <sweet/sweetmath.hpp>
#include <sweet/FatalError.hpp>


#define SPHERE_DATA_LON_CONTINUOUS	SHT_PHI_CONTIGUOUS
#define SPHERE_DATA_LAT_CONTINUOUS	SHT_THETA_CONTIGUOUS

// SWEET
//#define SPHERE_DATA_GRID_LAYOUT	SPHERE_DATA_LAT_CONTINUOUS

// SHTNS shallow water example
#define SPHERE_DATA_GRID_LAYOUT	SPHERE_DATA_LON_CONTINUOUS

class SphereDataConfig
{
	friend class SphereOperators;
	friend class SphereOperatorsComplex;
	friend class SphereData;
	friend class SphereDataComplex;

public:
	shtns_cfg shtns;

	/**
	 * Number of longitudes
	 */
public:
	int physical_num_lon;

	/**
	 * Number of latitudes
	 */
public:
	int physical_num_lat;

	/**
	 * Number of total longitudes and latitudes
	 */
public:
	int physical_array_data_number_of_elements;


	/**
	 * Number of modes
	 */
public:
	int spectral_modes_n_max;
	int spectral_modes_m_max;

	int fast_setup = sht_quick_init;
//	int fast_setup = 0;

//	double shtns_error = 1.e-10;
	double shtns_error = 0;

	/**
	 * Number of total number of modes (complex valued)
	 */
	int spectral_array_data_number_of_elements;

	/**
	 * Number of elements for SPH which is based on
	 * complex-valued physical data
	 */
	int spectral_complex_array_data_number_of_elements;


	/**
	 * Array with latitude phi angle values
	 *
	 * WARNING: Phi is not the phi from SHTNS!
	 */
public:
	double *lat;


	/**
	 * Array with mu = sin(phi) values
	 */
public:
	double *lat_gaussian;


	/**
	 * Array with comu = cos(phi) values
	 */
public:
	double *lat_cogaussian;



public:
	SphereDataConfig()	:
		shtns(nullptr),
		physical_num_lon(-1),
		physical_num_lat(-1),
		physical_array_data_number_of_elements(-1),

		spectral_modes_n_max(-1),
		spectral_modes_m_max(-1),
		spectral_array_data_number_of_elements(-1),
		spectral_complex_array_data_number_of_elements(-1),

		lat(nullptr),
		lat_gaussian(nullptr),
		lat_cogaussian(nullptr)
	{
	}

	const
	std::string getUniqueIDString()	const
	{
		return getConfigInformationString();
	}


	const
	std::string getConfigInformationString()	const
	{
		std::ostringstream buf;
		buf << "M" << spectral_modes_m_max << "," << spectral_modes_n_max << "_N" << physical_num_lon << "," << physical_num_lat;
		buf << " total_spec_modes: " << spectral_array_data_number_of_elements;
		return buf.str();
	}


	std::size_t getArrayIndexByModes(
			int n,
			int m
	)	const
	{
		assert(n >= 0);
		assert(n >= m);

//		return (spectral_modes_n_max-m)*m + ((m+1)*m)/2+n;
		return (m*(2*spectral_modes_n_max-m+1)>>1)+n;
	}



	std::size_t getArrayIndexByModes_Complex(
			int n,
			int m
	)	const
	{
		assert(n >= 0);
		assert(n >= std::abs(m));

		int idx = n*n+(m+n);
		return idx;
	}


	/**
	 * Return indices with N-variables compactly stored
	 */
	std::size_t getArrayIndexByModes_Complex_NCompact(
			int n,
			int m
	)	const
	{
		assert(n >= 0);
		assert(n >= std::abs(m));

		if (m < 0)
		{
			int minc = spectral_modes_m_max+m;
			int row_idx = (((minc-1)*minc)>>1) + minc;
			int idx =  row_idx + (n+m);
			return idx;
		}
		else
		{
			int rel_idx = (m*(2*spectral_modes_n_max-m+1)>>1)+n;
			return ((spectral_modes_n_max*(spectral_modes_n_max+1))>>1)+rel_idx;
		}
	}


private:
	void setup_data()
	{
		physical_num_lat = shtns->nlat;
		physical_num_lon = shtns->nphi;
		physical_array_data_number_of_elements = shtns->nspat;

		spectral_modes_n_max = shtns->lmax;
		spectral_modes_m_max = shtns->mmax;
		spectral_array_data_number_of_elements = shtns->nlm;
		spectral_complex_array_data_number_of_elements = (spectral_modes_n_max+1)*(spectral_modes_m_max+1);

		if (spectral_modes_n_max != spectral_modes_m_max)
		{
			std::cerr << "only spec_n_max == spec_m_max currently supported!" << std::endl;
			assert(false);
			exit(1);
		}

		/**
		 * Some safety checks to make sure that we really get what we've asked for
		 */

		/**
		 * TEST: iteration over the modes n,m for real-valued physical space
		 */
		{
			int idx = 0;
			for (int m = 0; m <= spectral_modes_m_max; m++)
			{

				int test_idx = getArrayIndexByModes(m,m);

				if (test_idx != idx)
				{
					std::cerr << "IDX TEST NOT SUCCESSFUL (real-valued physical transformation)" << std::endl;
					std::cout << "n=" << m << ", m=" << m << "     " << idx << ", " << test_idx << std::endl;
					exit(1);
				}

				for (int n = m; n <= spectral_modes_n_max; n++)
				{

					int test_idx2 = getArrayIndexByModes(n,m);
					if (test_idx2 != idx)
					{
						std::cerr << "IDX TEST2 NOT SUCCESSFUL (real-valued physical transformation)" << std::endl;
						std::cout << "n=" << n << ", m=" << m << "     " << idx << ", " << test_idx << std::endl;
						exit(1);
					}
					idx++;
				}
			}

			if (idx != spectral_array_data_number_of_elements)
			{
				std::cerr << "INTERNAL SPH ERROR (real-valued physical transformation)" << std::endl;
				assert(false);
				exit(1);
			}
		}



		/**
		 * TEST: iteration over the modes n,m for complex-valued physical space
		 *
		 * Note: In SHTNS, the m-coefficients are compactly stored for individual n'l
		 */
		{
			int idx = 0;
			for (int n = 0; n <= spectral_modes_n_max; n++)
			{
				int test_idx = getArrayIndexByModes_Complex(n,-n);

				if (test_idx != idx)
				{
					std::cerr << "IDX TEST NOT SUCCESSFUL (complex-valued physical transformation)" << std::endl;
					std::cout << "n=" << n << ", m=" << -n << "     " << idx << ", " << test_idx << std::endl;
					exit(1);
				}


				for (int m = -n; m <= n; m++)
				{
//					std::cout << std::endl;
//					std::cout << "TESTING inner loop n= " << n << ", m = " << m << std::endl;

					int test_idx2 = getArrayIndexByModes_Complex(n,m);

					if (test_idx2 != idx)
					{
						std::cerr << "IDX TEST2 NOT SUCCESSFUL (complex-valued physical transformation)" << std::endl;
						std::cout << "n=" << n << ", m=" << m << "     " << idx << ", " << test_idx << std::endl;
						exit(1);
					}

					idx++;
				}
			}


			if (idx != spectral_complex_array_data_number_of_elements)
			{
				std::cerr << "INTERNAL SPH ERROR" << std::endl;
				assert(false);
				exit(1);
			}
		}


		/**
		 * TEST: iteration over the modes n,m for complex-valued physical space
		 *
		 * This version tests for the n-coefficients compactly stored for individual m's
		 */
		{
			int idx = 0;
			for (int m = -spectral_modes_m_max; m <= spectral_modes_m_max; m++)
			{
				int test_idx = getArrayIndexByModes_Complex_NCompact(std::abs(m),m);

				if (test_idx != idx)
				{
					std::cerr << "IDX TEST NOT SUCCESSFUL (complex-valued physical transformation)" << std::endl;
					std::cout << "n=" << m << ", m=" << m << "     " << idx << ", " << test_idx << std::endl;
					exit(1);
				}


				for (int n = std::abs(m); n <= spectral_modes_n_max; n++)
				{
					int test_idx2 = getArrayIndexByModes_Complex_NCompact(n,m);

					if (test_idx2 != idx)
					{
						std::cerr << "IDX TEST2 NOT SUCCESSFUL (complex-valued physical transformation)" << std::endl;
						std::cout << "n=" << n << ", m=" << m << "     " << idx << ", " << test_idx << std::endl;
						exit(1);
					}

					idx++;
				}
			}
		}

		lat = (double*)fftw_malloc(sizeof(double)*physical_num_lat);

		/*
		 * Colatitude is 0 at the north pole and 180 at the south pole
		 *
		 * WARNING: The latitude degrees are not aequidistnat spaced in the angles!!!!
		 * Those points are computed for optimal Gauss quadrature.
		 * They are close to aequidistnat spacing, but not fully aequidistant.
		 *
		 * We have to use the shtns->ct lookup table
		 */
		for (int i = 0; i < physical_num_lat; i++)
			lat[i] = M_PI_2 - std::acos(shtns->ct[i]);

		lat_gaussian = (double*)fftw_malloc(sizeof(double)*shtns->nlat);
		for (int i = 0; i < physical_num_lat; i++)
			lat_gaussian[i] = shtns->ct[i];		/// sin(phi) (SHTNS stores cos(phi))

		lat_cogaussian = (double*)fftw_malloc(sizeof(double)*shtns->nlat);
		for (int i = 0; i < physical_num_lat; i++)
			lat_cogaussian[i] = shtns->st[i];	/// cos(phi) (SHTNS stores sin(phi))

#if 0
		getConfigInformationString();
		std::cout << "physical_num_lat: " << physical_num_lat << std::endl;
		for (int i = 0; i < physical_num_lat; i++)
			std::cout << i << ": " << asin(lat_gaussian[i]) << std::endl;
#endif
	}



public:
	void setup(
			int nphi,	// physical
			int nlat,

			int mmax,	// spectral
			int nmax

	)
	{
		cleanup();

		shtns_verbose(0);			// displays informations during initialization.

		// enable multi-threaded transforms (if supported).
#if SWEET_SPACE_THREADING
		shtns_use_threads(0);	// automatically choose number of threads
#else
		shtns_use_threads(1);	// value of 1 disables threading
#endif

		if (shtns != nullptr)
			shtns_destroy(shtns);

		shtns = shtns_create(
				nmax,
				mmax,
				1,
				(shtns_norm)((int)sht_orthonormal + SHT_NO_CS_PHASE)
			);

		shtns_set_grid(
				shtns,
				(shtns_type)(fast_setup | SPHERE_DATA_GRID_LAYOUT),
				//sht_gauss | SHT_THETA_CONTIGUOUS,	// use gaussian grid
				shtns_error,
				nlat,		// number of latitude grid points
				nphi		// number of longitude grid points
			);

		setup_data();
	}



	/**
	 * Setup with given modes.
	 * Spatial resolution will be determined automatically
	 */
	void setupAutoPhysicalSpace(
			int i_mmax,		/// longitude modes
			int i_nmax,		/// latitude modes
			int *o_nphi,	/// physical resolution along longitude
			int *o_nlat		/// physical resolution along latitude
	)
	{
		cleanup();

		shtns_verbose(0);			// displays informations during initialization.
#if SWEET_SPACE_THREADING
		shtns_use_threads(0);	// automatically choose number of threads
#else
		shtns_use_threads(1);	// value of 1 disables threading
#endif


		if (shtns != nullptr)
			shtns_destroy(shtns);

		shtns = shtns_create(
				i_nmax,
				i_mmax,
				1,
				(shtns_norm)((int)sht_orthonormal + SHT_NO_CS_PHASE)
			);

		*o_nphi = 0;
		*o_nlat = 0;

		shtns_set_grid_auto(
				shtns,
				// TODO: replace this with sht_gauss
				(shtns_type)(fast_setup | SPHERE_DATA_GRID_LAYOUT),
				//sht_gauss | SPHERE_DATA_GRID_LAYOUT,	// use gaussian grid
				shtns_error,
				2,		// use order 2
				o_nlat,
				o_nphi
			);

		setup_data();
	}




	/**
	 * Setup with given modes.
	 * Spatial resolution will be determined automatically
	 */
	void setupAutoPhysicalSpace(
			int i_mmax,		/// longitude modes
			int i_nmax		/// latitude modes
	)
	{
		cleanup();

		shtns_verbose(0);			// displays informations during initialization.
#if SWEET_SPACE_THREADING
		shtns_use_threads(0);	// automatically choose number of threads
#else
		shtns_use_threads(1);	// value of 1 disables threading
#endif


		if (shtns != nullptr)
			shtns_destroy(shtns);

		shtns = shtns_create(
				i_nmax,
				i_mmax,
				1,
				(shtns_norm)((int)sht_orthonormal + SHT_NO_CS_PHASE)
			);

		physical_num_lat = 0;
		physical_num_lon = 0;

		shtns_set_grid_auto(
				shtns,
				// TODO: replace this with sht_gauss
				(shtns_type)(fast_setup | SPHERE_DATA_GRID_LAYOUT),
				//sht_gauss | SPHERE_DATA_GRID_LAYOUT,	// use gaussian grid
				shtns_error,
				2,		// use order 2
				&physical_num_lat,
				&physical_num_lon
			);

		setup_data();
	}



public:
	void setupAuto(
			int io_physical_res[2],
			int io_spectral_modes[2]
	)
	{
		cleanup();

//		std::cout << io_physical_res[0] << ", " << io_physical_res[1] << std::endl;
//		std::cout << io_spectral_modes[0] << ", " << io_spectral_modes[1] << std::endl;

		if (io_physical_res[0] > 0 && io_spectral_modes[0] > 0)
		{
			setup(	io_physical_res[0],
					io_physical_res[1],
					io_spectral_modes[0],
					io_spectral_modes[1]
				);
			return;
		}

		if (io_physical_res[0] > 0)
		{
			FatalError("TODO: Automatic spectral space mode computation");
#if 0
			setupAutoSpectralSpace(
					io_physical_res[0],
					io_physical_res[1]
				);

	#if SWEET_USE_LIBFFT
			io_spectral_modes[0] = spectral_modes[0];
			io_spectral_modes[1] = spectral_modes[1];
	#endif
#endif
			return;
		}

		if (io_spectral_modes[0] > 0)
		{
#if SWEET_USE_LIBFFT
			setupAutoPhysicalSpace(
					io_spectral_modes[0],
					io_spectral_modes[1]
				);

			io_physical_res[0] = physical_num_lon;
			io_physical_res[1] = physical_num_lat;
#else
			FatalError("Setup with spectral modes not enabled");
#endif
			return;
		}

		FatalError("No resolution/modes selected");
	}




	void setupAdditionalModes(
			SphereDataConfig *i_sphereDataConfig,
			int i_additional_modes_longitude,
			int i_additional_modes_latitude
	)
	{
		cleanup();

		assert(shtns == nullptr);

		setupAutoPhysicalSpace(
				i_sphereDataConfig->spectral_modes_m_max + i_additional_modes_longitude,
				i_sphereDataConfig->spectral_modes_n_max + i_additional_modes_latitude,
				&physical_num_lon,
				&physical_num_lat
		);
	}



	void cleanup()
	{
		// check if sphereDataConfig was initialized
		if (shtns == nullptr)
			return;

		shtns_destroy(shtns);
		shtns = nullptr;

		fftw_free(lat);
		lat = nullptr;

		fftw_free(lat_gaussian);
		lat_gaussian = nullptr;

		fftw_free(lat_cogaussian);
		lat_cogaussian = nullptr;

#if SWEET_USE_THREADING
		fftw_cleanup_threads();
#endif
		fftw_cleanup();
	}



	~SphereDataConfig()
	{
		cleanup();
	}
};



#endif /* SPHSETUP_HPP_ */
