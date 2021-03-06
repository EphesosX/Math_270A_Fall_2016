/**
Copyright (c) 2016 Theodore Gast, Chuyuan Fu, Chenfanfu Jiang, Joseph Teran

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

If the code is used in an article, the following paper shall be cited:
@techreport{qrsvd:2016,
  title={Implicit-shifted Symmetric QR Singular Value Decomposition of 3x3 Matrices},
  author={Gast, Theodore and Fu, Chuyuan and Jiang, Chenfanfu and Teran, Joseph},
  year={2016},
  institution={University of California Los Angeles}
}

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cmath>
#include "Tools.h"
#include "ImplicitQRSVD.h"

template <class T>
void testAccuracy(const std::vector<Eigen::Matrix<T, 3, 3> >& AA,
    const std::vector<Eigen::Matrix<T, 3, 3> >& UU,
    const std::vector<Eigen::Matrix<T, 3, 1> >& SS,
    const std::vector<Eigen::Matrix<T, 3, 3> >& VV)
{
    T max_UUt_error = 0, max_VVt_error = 0, max_detU_error = 0, max_detV_error = 0, max_reconstruction_error = 0;
    T ave_UUt_error = 0, ave_VVt_error = 0, ave_detU_error = 0, ave_detV_error = 0, ave_reconstruction_error = 0;
    for (size_t i = 0; i < AA.size(); i++) {
        Eigen::Matrix<T, 3, 3> M = AA[i];
        Eigen::Matrix<T, 3, 1> S = SS[i];
        Eigen::Matrix<T, 3, 3> U = UU[i];
        Eigen::Matrix<T, 3, 3> V = VV[i];
        T error;
        error = (U * U.transpose() - Eigen::Matrix<T, 3, 3>::Identity()).array().abs().maxCoeff();
        max_UUt_error = (error > max_UUt_error) ? error : max_UUt_error;
        ave_UUt_error += fabs(error);
        error = (V * V.transpose() - Eigen::Matrix<T, 3, 3>::Identity()).array().abs().maxCoeff();
        max_VVt_error = (error > max_VVt_error) ? error : max_VVt_error;
        ave_VVt_error += fabs(error);
        error = fabs(fabs(U.determinant()) - (T)1);
        max_detU_error = (error > max_detU_error) ? error : max_detU_error;
        ave_detU_error += fabs(error);
        error = fabs(fabs(V.determinant()) - (T)1);
        max_detV_error = (error > max_detV_error) ? error : max_detV_error;
        ave_detV_error += fabs(error);
        error = (U * S.asDiagonal() * V.transpose() - M).array().abs().maxCoeff();
        max_reconstruction_error = (error > max_reconstruction_error) ? error : max_reconstruction_error;
        ave_reconstruction_error += fabs(error);
    }
    ave_UUt_error /= (T)(AA.size());
    ave_VVt_error /= (T)(AA.size());
    ave_detU_error /= (T)(AA.size());
    ave_detV_error /= (T)(AA.size());
    ave_reconstruction_error /= (T)(AA.size());
    std::cout << std::setprecision(10) << " UUt max error: " << max_UUt_error
              << " VVt max error: " << max_VVt_error
              << " detU max error:" << max_detU_error
              << " detV max error:" << max_detV_error
              << " recons max error:" << max_reconstruction_error << std::endl;
    std::cout << std::setprecision(10) << " UUt ave error: " << ave_UUt_error
              << " VVt ave error: " << ave_VVt_error
              << " detU ave error:" << ave_detU_error
              << " detV ave error:" << ave_detV_error
              << " recons ave error:" << ave_reconstruction_error << std::endl;
}

template <class T>
void runImplicitQRSVD(const int repeat, const std::vector<Eigen::Matrix<T, 3, 3> >& tests, const bool accuracy_test)
{
    using namespace JIXIE;
    std::vector<Eigen::Matrix<T, 3, 3> > UU, VV;
    std::vector<Eigen::Matrix<T, 3, 1> > SS;
    JIXIE::Timer timer;
    timer.start();
    double total_time = 0;
    for (int test_iter = 0; test_iter < repeat; test_iter++) {
        timer.click();
        for (size_t i = 0; i < tests.size(); i++) {
            Eigen::Matrix<T, 3, 3> M = tests[i];
            Eigen::Matrix<T, 3, 1> S;
            Eigen::Matrix<T, 3, 3> U;
            Eigen::Matrix<T, 3, 3> V;
            singularValueDecomposition(M, U, S, V);
            if (accuracy_test && test_iter == 0) {
                UU.push_back(U);
                SS.push_back(S);
                VV.push_back(V);
            }
        }
        double this_time = timer.click();
        total_time += this_time;
        std::cout << std::setprecision(10) << "impQR time: " << this_time << std::endl;
    }
    std::cout << std::setprecision(10) << "impQR Average time: " << total_time / (double)(repeat) << std::endl;
    if (accuracy_test)
        testAccuracy(tests, UU, SS, VV);
}

template <class T>
void addRandomCases(std::vector<Eigen::Matrix<T, 3, 3> >& tests, const T random_range, const int N)
{
    using namespace JIXIE;
    int old_count = tests.size();
    std::cout << std::setprecision(10) << "Adding random test cases with range " << -random_range << " to " << random_range << std::endl;
    RandomNumber<T> random_gen(123);
    for (int t = 0; t < N; t++) {
        Eigen::Matrix<T, 3, 3> Z;
        random_gen.fill(Z, -random_range, random_range);
        tests.push_back(Z);
    }
    std::cout << std::setprecision(10) << tests.size() - old_count << " cases added." << std::endl;
    std::cout << std::setprecision(10) << "Total test cases: " << tests.size() << std::endl;
}

template <class T>
void addIntegerCases(std::vector<Eigen::Matrix<T, 3, 3> >& tests, const int int_range)
{
    using namespace JIXIE;
    int old_count = tests.size();
    std::cout << std::setprecision(10) << "Adding integer test cases with range " << -int_range << " to " << int_range << std::endl;
    Eigen::Matrix<T, 3, 3> Z;
    Z.fill(-int_range);
    typename Eigen::Matrix<T, 3, 3>::Index i = 0;
    tests.push_back(Z);
    while (i < Eigen::Matrix<T, 3, 3>::SizeAtCompileTime) {
        if (Z(i) < int_range) {
            Z(i)++;
            tests.push_back(Z);
            i = 0;
        }
        else {
            Z(i) = -int_range;
            i++;
        }
    }
    std::cout << std::setprecision(10) << tests.size() - old_count << " cases added." << std::endl;
    std::cout << std::setprecision(10) << "Total test cases: " << tests.size() << std::endl;
}

template <class T>
void addPerturbationFromIdentityCases(std::vector<Eigen::Matrix<T, 3, 3> >& tests, const int num_perturbations, const T perturb)
{
    using namespace JIXIE;
    int old_count = tests.size();
    std::vector<Eigen::Matrix<T, 3, 3> > tests_tmp;
    Eigen::Matrix<T, 3, 3> Z = Eigen::Matrix<T, 3, 3>::Identity();
    tests_tmp.push_back(Z);
    std::cout << std::setprecision(10) << "Adding perturbed identity test cases with perturbation " << perturb << std::endl;
    RandomNumber<T> random_gen(123);
    size_t special_cases = tests_tmp.size();
    for (size_t t = 0; t < special_cases; t++) {
        for (int i = 0; i < num_perturbations; i++) {
            random_gen.fill(Z, -perturb, perturb);
            tests.push_back(tests_tmp[t] + Z);
        }
    }
    std::cout << std::setprecision(10) << tests.size() - old_count << " cases added." << std::endl;
    std::cout << std::setprecision(10) << "Total test cases: " << tests.size() << std::endl;
}

template <class T>
void addPerturbationCases(std::vector<Eigen::Matrix<T, 3, 3> >& tests, const int int_range, const int num_perturbations, const T perturb)
{
    using namespace JIXIE;
    int old_count = tests.size();
    std::vector<Eigen::Matrix<T, 3, 3> > tests_tmp;
    Eigen::Matrix<T, 3, 3> Z;
    Z.fill(-int_range);
    typename Eigen::Matrix<T, 3, 3>::Index i = 0;
    tests_tmp.push_back(Z);
    while (i < Eigen::Matrix<T, 3, 3>::SizeAtCompileTime) {
        if (Z(i) < int_range) {
            Z(i)++;
            tests_tmp.push_back(Z);
            i = 0;
        }
        else {
            Z(i) = -int_range;
            i++;
        }
    }
    std::cout << std::setprecision(10) << "Adding perturbed integer test cases with perturbation " << perturb << " and range " << -int_range << " to " << int_range << std::endl;
    RandomNumber<T> random_gen(123);
    size_t special_cases = tests_tmp.size();
    for (size_t t = 0; t < special_cases; t++) {
        for (int i = 0; i < num_perturbations; i++) {
            random_gen.fill(Z, -perturb, perturb);
            tests.push_back(tests_tmp[t] + Z);
        }
    }
    std::cout << std::setprecision(10) << tests.size() - old_count << " cases added." << std::endl;
    std::cout << std::setprecision(10) << "Total test cases: " << tests.size() << std::endl;
}

void runBenchmark()
{
    using namespace JIXIE;
    using std::fabs;

    bool run_qr;

    bool test_float;
    bool test_double;
    bool accuracy_test;
    bool normalize_matrix;
    int number_of_repeated_experiments;
    bool test_random;
    int random_range;
    int number_of_random_cases;
    bool test_integer;
    int integer_range;
    bool test_perturbation;
    int perturbation_count;
    float float_perturbation;
    double double_perturbation;
    bool test_perturbation_from_identity;
    int perturbation_from_identity_count;
    float float_perturbation_identity;
    double double_perturbation_identity;
    std::string title;

    // Finalized options
    run_qr = true;

    test_float = true;
    test_double = true;
    normalize_matrix = false;
    int number_of_repeated_experiments_for_timing = 2;

    for (int test_number = 1; test_number <= 10; test_number++) {

        if (test_number == 1) {
            title = "random timing test";
            number_of_repeated_experiments = number_of_repeated_experiments_for_timing;
            accuracy_test = false;
            test_random = true, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, integer_range = 3, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = false, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 2) {
            title = "integer timing test";
            number_of_repeated_experiments = number_of_repeated_experiments_for_timing;
            accuracy_test = false;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = true; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = false, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 3) {
            title = "integer-perturbation timing test: 256 eps";
            number_of_repeated_experiments = number_of_repeated_experiments_for_timing;
            accuracy_test = false;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = true, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = false, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 4) {
            title = "identity-perturbation timing test: 1e-3";
            number_of_repeated_experiments = number_of_repeated_experiments_for_timing;
            accuracy_test = false;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = true, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 5) {
            title = "identity-perturbation timing test: 256 eps";
            number_of_repeated_experiments = number_of_repeated_experiments_for_timing;
            accuracy_test = false;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = true, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation_identity = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed itentity test
        }

        if (test_number == 6) {
            title = "random accuracy test";
            number_of_repeated_experiments = 1;
            accuracy_test = true;
            test_random = true, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, integer_range = 3, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = false, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 7) {
            title = "integer accuracy test";
            number_of_repeated_experiments = 1;
            accuracy_test = true;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = true; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = false, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 8) {
            title = "integer-perturbation accuracy test: 256 eps";
            number_of_repeated_experiments = 1;
            accuracy_test = true;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = true, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = false, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 9) {
            title = "identity-perturbation accuracy test: 1e-3";
            number_of_repeated_experiments = 1;
            accuracy_test = true;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = true, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = 1e-3, double_perturbation_identity = 1e-3; // perturbed itentity test
        }
        if (test_number == 10) {
            title = "identity-perturbation accuracy test: 256 eps";
            number_of_repeated_experiments = 1;
            accuracy_test = true;
            test_random = false, random_range = 3, number_of_random_cases = 1024 * 1024; // random test
            test_integer = false; // integer test
            integer_range = 2; // this variable is used by both integer test and perturbed integer test
            test_perturbation = false, perturbation_count = 4, float_perturbation = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed integer test
            test_perturbation_from_identity = true, perturbation_from_identity_count = 1024 * 1024, float_perturbation_identity = (float)256 * std::numeric_limits<float>::epsilon(), double_perturbation_identity = (double)256 * std::numeric_limits<double>::epsilon(); // perturbed itentity test
        }

        std::cout << " \n========== RUNNING BENCHMARK TEST == " << title << "=======" << std::endl;
        std::cout << " run_qr " << run_qr << std::endl;
        std::cout << " test_float " << test_float << std::endl;
        std::cout << " test_double " << test_double << std::endl;
        std::cout << " accuracy_test " << accuracy_test << std::endl;
        std::cout << " normalize_matrix " << normalize_matrix << std::endl;
        std::cout << " number_of_repeated_experiments " << number_of_repeated_experiments << std::endl;
        std::cout << " test_random " << test_random << std::endl;
        std::cout << " random_range " << random_range << std::endl;
        std::cout << " number_of_random_cases " << number_of_random_cases << std::endl;
        std::cout << " test_integer " << test_integer << std::endl;
        std::cout << " integer_range " << integer_range << std::endl;
        std::cout << " test_perturbation " << test_perturbation << std::endl;
        std::cout << " perturbation_count " << perturbation_count << std::endl;
        std::cout << " float_perturbation " << float_perturbation << std::endl;
        std::cout << " double_perturbation " << double_perturbation << std::endl;
        std::cout << " test_perturbation_from_identity " << test_perturbation_from_identity << std::endl;
        std::cout << " perturbation_from_identity_count " << perturbation_from_identity_count << std::endl;
        std::cout << " float_perturbation_identity " << float_perturbation_identity << std::endl;
        std::cout << " double_perturbation_identity " << double_perturbation_identity << std::endl;

        std::cout << std::setprecision(10) << "\n--- float test ---\n" << std::endl;
        if (test_float) {
            std::vector<Eigen::Matrix<float, 3, 3> > tests;
            if (test_integer)
                addIntegerCases(tests, integer_range);
            if (test_perturbation)
                addPerturbationCases(tests, integer_range, perturbation_count, float_perturbation);
            if (test_perturbation_from_identity)
                addPerturbationFromIdentityCases(tests, perturbation_from_identity_count, float_perturbation_identity);
            if (test_random)
                addRandomCases(tests, (float)random_range, number_of_random_cases);
            if (normalize_matrix) {
                for (size_t i = 0; i < tests.size(); i++) {
                    float norm = tests[i].norm();
                    if (norm > (float)8 * std::numeric_limits<float>::epsilon()) {
                        tests[i] /= norm;
                    }
                }
            }
            std::cout << std::setprecision(10) << "\n-----------" << std::endl;
            if (run_qr)
                runImplicitQRSVD(number_of_repeated_experiments, tests, accuracy_test);

        }

        std::cout << std::setprecision(10) << "\n--- double test ---\n" << std::endl;
        if (test_double) {
            std::vector<Eigen::Matrix<double, 3, 3> > tests;
            if (test_integer)
                addIntegerCases(tests, integer_range);
            if (test_perturbation)
                addPerturbationCases(tests, integer_range, perturbation_count, double_perturbation);
            if (test_perturbation_from_identity)
                addPerturbationFromIdentityCases(tests, perturbation_from_identity_count, double_perturbation_identity);
            if (test_random)
                addRandomCases(tests, (double)random_range, number_of_random_cases);
            if (normalize_matrix) {
                for (size_t i = 0; i < tests.size(); i++) {
                    double norm = tests[i].norm();
                    if (norm > (double)8 * std::numeric_limits<double>::epsilon()) {
                        tests[i] /= norm;
                    }
                }
            }
            std::cout << std::setprecision(10) << "\n-----------" << std::endl;
            if (run_qr)
                runImplicitQRSVD(number_of_repeated_experiments, tests, accuracy_test);

        }
    }
}

// Jacobi rotation from JIXIE for comparison to Eigen
void JIXIE_JacobiRotation(const Eigen::Matrix2f& S_Sym, Eigen::Matrix2f& sigma, float& cosine, float& sine) {
	typedef float T;
	T x = S_Sym(0, 0);
	T y = S_Sym(0, 1);
	T z = S_Sym(1, 1);
	if (y == 0) {
		// S is already diagonal
		cosine = 1;
		sine = 0;
		sigma(0,0) = x;
		sigma(1,1) = z;
	} else {
		T tau = 0.5 * (x - z);
		T w = sqrt(tau * tau + y * y);
		// w > y > 0
		T t;
		if (tau > 0) {
			// tau + w > w > y > 0 ==> division is safe
			t = y / (tau + w);
		} else {
			// tau - w < -w < -y < 0 ==> division is safe
			t = y / (tau - w);
		}
		cosine = T(1) / sqrt(t * t + T(1));
		sine = -t * cosine;
		/*
		V = [cosine -sine; sine cosine]
		Sigma = V'SV. Only compute the diagonals for efficiency.
		Also utilize symmetry of S and don't form V yet.
		*/
		T c2 = cosine * cosine;
		T csy = 2 * cosine * sine * y;
		T s2 = sine * sine;
		sigma(0,0) = c2 * x - csy + s2 * z;
		sigma(1,1) = s2 * x + csy + c2 * z;
	}
}


void My_SVD(const Eigen::Matrix2f& F,Eigen::Matrix2f& U,Eigen::Matrix2f& sigma,Eigen::Matrix2f& V){
//
//Compute the SVD of input F with sign conventions discussed in class and in assignment
//
//input: F
//output: U,sigma,V with F=U*sigma*V.transpose() and U*U.transpose()=V*V.transpose()=I;

	//bool swapped = false;
	// C=F' F
	Eigen::Matrix2f C;
	C.noalias() = F.transpose()*F;
	
	// Create Jacobi rotation
	
	/*Eigen::JacobiRotation<float> J;
	J.makeJacobi(C, 0, 1);
	sigma.noalias() = C;
	sigma.applyOnTheLeft(0, 1, J.transpose());
	sigma.applyOnTheRight(0, 1, J);
	V.noalias() = Eigen::Matrix2f::Identity();
	V.applyOnTheLeft(0, 1, J);*/

	

	sigma.noalias() = Eigen::Matrix2f::Zero();
	float cosine, sine;
	JIXIE_JacobiRotation(C, sigma, cosine, sine);
	V << cosine, sine, -sine, cosine;

	/*if (sigma(0, 0) > sigma(1, 1)) {
		std::cout << sigma(0, 0) - 25 << " " << sigma(1, 1) << std::endl;
	} else {
		std::cout << sigma(1, 1) - 25 << " " << sigma(0, 0) << std::endl;
	}*/
	
	// square root singular values
	if (sigma(0, 0) > 0) { // check for floating point issues near 0
		sigma(0, 0) = std::sqrt(sigma(0, 0));
	} else {
		sigma(0, 0) = 0;
	}
	if (sigma(1, 1) > 0) {
		sigma(1, 1) = std::sqrt(sigma(1, 1));
	} else {
		sigma(1, 1) = 0;
	}
	sigma(0, 1) = 0;
	sigma(1, 0) = 0;
	//std::cout << sigma << std::endl;
	//std::cout << sigma(1, 1) << std::endl;
	
	
	/*std::cout << V << std::endl;
	std::cout << sigma << std::endl;
	std::cout << V*sigma*sigma*V.transpose()<<std::endl;*/
	//std::cout << "Swapping" << std::endl;

	// sort singular values and permute columns
	if (sigma(0, 0) < sigma(1, 1)) {
		Eigen::Matrix2f P;
		P << 0, 1, 1, 0;
		// swap the sigmas
		sigma = P*sigma*P;
		// swap columns of J
		V *= P;
		//swapped = true;
	}

	//std::cout << V << std::endl;
	//std::cout << sigma << std::endl;
	//std::cout << V*sigma*sigma*V.transpose() << std::endl;

	// A = F V
	Eigen::Matrix2f A;
	A.noalias() = F*V;
	//std::cout << "A:" << A << std::endl;
	// Givens(a11,a22,c,s)
	Eigen::JacobiRotation<float> U2;
	U2.makeGivens(A(0,0), A(1,0));
	U << 1, 0, 0, 1;
	U.applyOnTheLeft(0,1,U2);
	//std::cout << U << std::endl;
	//Eigen::Matrix2f R;
	//R.noalias() = U.transpose() * A;
	//std::cout << R << std::endl;
	// U=[c,s;-s,c]
	
	// sigma2 = a12*s + a22*c;
	float sigma2 = A(0,1) * U(0,1) + A(1,1) * U(0,0);
	//std::cout << sigma2 << std::endl;
	sigma(1, 1) = sigma2;
	// Fix signs at the end
	
	if (V.determinant() < 0) {
		sigma(1, 1) = -sigma(1, 1);
		Eigen::Matrix2f P2;
		P2 << 1, 0, 0, -1; 
		V *= P2;
	}
}

void My_Polar(const Eigen::Matrix3f& F,Eigen::Matrix3f& R,Eigen::Matrix3f& S){
  //
  //Compute the polar decomposition of input F (with det(R)=1)
  //
  //input: F
  //output: R,s with F=R*S and R*R.transpose()=I and S=S.transpose()
	int max_it = 1000;
	float tol = 0.0000001;

	//R=I
	R.noalias() = Eigen::Matrix3f::Identity();
	//S=F
	S.noalias() = F;
	//it=0
	int it = 0;
	//while(it<max_it || max(abs(S_21-S_12), abs(S_31-S_13),abs(S_23-S_32))>tol) {
	while (it < max_it && std::max(std::max(std::fabs(S(1, 0) - S(0, 1)), std::fabs(S(2, 0) - S(0, 2))), std::fabs(S(1, 2) - S(2, 1)))>tol) {
		for (int i = 0; i <= 2; i++) {
			for (int j = i+1; j <= 2; j++) {
				//Givens(S_ii+S_jj,S_ji-S_ij,b_ij,c,s)
				//std::cout << "i: " << i << " j: " << j << std::endl;
				float trace = S(i, i) + S(j, j);
				float sym_diff = S(j, i) - S(i, j);
				//std::cout << trace << std::endl;
				//std::cout << sym_diff << std::endl;
				JIXIE::GivensRotation<float> G(trace, sym_diff, i, j);
				//Eigen::Matrix3f G2;
				//G.fill(G2);
				//std::cout << G2 << std::endl;
				
				//R=R G_ij
				//R *= G(i, j);
				G.columnRotation(R);
				
				//std::cout << R << std::endl;
				
				/*float denom = JIXIE::MATH_TOOLS::rsqrt(trace*trace + sym_diff*sym_diff);
				S(i, j) = (S(i, i)*S(i, j) + S(j, j)*S(j, i))*denom;
				S(j, i) = S(i, j);
				S(i, i) = (S(i, i)*S(i, i) + S(i, i)*S(j, j) - S(i, j)*S(j, i) + S(j, i)*S(j, i))*denom;
				S(j, j) = (S(j, j)*S(j, j) + S(j, j)*S(i, i) - S(i, j)*S(j, i) + S(i, j)*S(i, j))*denom;
*/
				G.rowRotation(S);
				//std::cout << S << std::endl;
				//std::cout << R*S << std::endl;
			}
		}
		it++;
	}
}

void Algorithm_2_Test(int nTrials, float tol) {

	Eigen::Matrix2f sigmai;
	float sigma1 = 5; float sigma2 = 0;
	sigmai << sigma1, 0, 0, sigma2;
	srand(time(NULL));
	for (int i = 0; i < nTrials; i++) {
		Eigen::Matrix2f F, U, sigma, V;
		Eigen::Matrix2f Ui, Vi;
		double theta1 = (double)rand() / RAND_MAX * 2 * 3.1415926535897;
		double theta2 = (double)rand() / RAND_MAX * 2 * 3.1415926535897;
		Ui << cos(theta1), sin(theta1), -sin(theta1), cos(theta1);
		Vi << cos(theta2), sin(theta2), -sin(theta2), cos(theta2);
		F.noalias() = Ui * sigmai * Vi.transpose();
		My_SVD(F, U, sigma, V);
		Eigen::Matrix2f Fdiff;
		Fdiff.noalias() = F - U * sigma * V.transpose();
		if (std::abs(Fdiff(0, 0)) > tol || std::abs(Fdiff(1, 0)) > tol || std::abs(Fdiff(0, 1)) > tol || std::abs(Fdiff(1, 1)) > tol) {
			std::cout << "Tolerance not met" << std::endl;
			std::cout << Fdiff << std::endl;
			std::cout << std::abs(sigma(0, 0) - sigma1) << " " << std::abs(sigma(1, 1) - sigma2) << std::endl;
			std::cout << U << std::endl;
			std::cout << V << std::endl;
			std::cout << F << std::endl;
			std::cout << U*sigma*V.transpose() << std::endl;
			std::cout << sigma << std::endl;
		}
		if (U.determinant() < 0 || V.determinant() < 0) {
			std::cout << "Determinants not properly signed" << std::endl;
			std::cout << U << std::endl;
			std::cout << V << std::endl;
			std::cout << V.determinant() << std::endl;
			std::cout << F << std::endl;
			std::cout << F.determinant() << std::endl;
			std::cout << U*sigma*V.transpose() << std::endl;
			std::cout << sigma << std::endl;
		}
	}
	std::cout << nTrials << " trials run." << std::endl;
}

int main()
{
  bool run_benchmark = false;
  if (run_benchmark) runBenchmark();
  //Algorithm_2_Test(1000,0.000001);
  Eigen::Matrix3f F, R, S;
  F << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  My_Polar(F, R, S);
  Eigen::Matrix3f F2;
  F2 = R * S;
  std::cout << F2 << std::endl;
  std::cout << R << std::endl;
  std::cout << S << std::endl;
  std::cout << R*R.transpose() << std::endl;
}
