//
// Created by James on 2019/4/2.
//

#ifndef PHAROSA_LINEAR_HPP
#define PHAROSA_LINEAR_HPP

// Linear algebra solver

#include "../lib.h"

// Solve A b == x (n_dim == n, A: matrix)
bool SolveLinear(double *A, double *b, double *x, int n)
{
	double M[n][n + 1];        // expand to (n+1) cols
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			M[i][j] = (A + i * n)[j];
		}
		M[i][n] = b[i];
	}
	for (int k = 0; k < n; ++k) {    // n principle component
		double col_max = fabs(M[k][k]), fm;
		int maxi = k;
		for (int i = k + 1; i < n; ++i) {    // find the max in k-th col
			if ((fm = fabs(M[i][k])) > col_max) {
				col_max = fm;
				maxi = i;
			}
		}
		if (col_max < EPS) {        // singular matrix
			return false;        // solving failed
		}
		// swap k-th and maxi-th row
		for (int j = k; j < n + 1; ++j) {
			std::swap(M[k][j], M[maxi][j]);
		}

		// perform reduction
		for (int i = k + 1; i < n; ++i) {
			double r = -M[i][k] / M[k][k];
			for (int j = k + 1; j < n + 1; ++j) {
				M[i][j] += r * M[k][j];
			}
		}
	}
	// regressive solving
	for (int k = n - 1; k >= 0; --k) {
		for (int i = 0; i < k; ++i) {
			M[i][n] += -M[i][k] / M[k][k] * M[k][n];
		}
		x[k] = M[k][n] / M[k][k];
	}
	return true;    // solving success
}

// Solve A b == x (n_dim == n, A: matrix) for debuging use
bool SolveLinearDebug(double *A, double *b, double *x, int n)
{
	double M[n][n + 1];  // expand to (n+1) cols
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			M[i][j] = (A + i * n)[j];
		}
		M[i][n] = b[i];
	}
	debug("before reduction:\n");
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n + 1; ++j) {
			debug("%.2f ", M[i][j]);
		}
		debug("\n");
	}
	debug("\n");

	for (int k = 0; k < n; ++k) {    // n principle component
		double col_max = fabs(M[k][k]), fm;
		int maxi = k;
		for (int i = k + 1; i < n; ++i) {    // find the max in k-th col
			if ((fm = fabs(M[i][k])) > col_max) {
				col_max = fm;
				maxi = i;
			}
		}
		if (col_max < EPS) {        // singular matrix
			return false;        // solving failed
		}
		// swap k-th and maxi-th row
		for (int j = k; j < n + 1; ++j) {
			std::swap(M[k][j], M[maxi][j]);
		}

		// perform reduction
		for (int i = k + 1; i < n; ++i) {
			double r = -M[i][k] / M[k][k];
			M[i][k] = 0;
			for (int j = k + 1; j < n + 1; ++j) {
				M[i][j] += r * M[k][j];
			}
		}
	}
	debug("after reduction:\n");
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n + 1; ++j) {
			debug("%.2f ", M[i][j]);
		}
		debug("\n");
	}
	debug("\n");

	// regressive solving
	for (int k = n - 1; k >= 0; --k) {
		for (int i = 0; i < k; ++i) {
			M[i][n] += -M[i][k] / M[k][k] * M[k][n];
			M[i][k] = 0;
		}
		M[k][n] /= M[k][k];
		M[k][k] = 1;
		x[k] = M[k][n];
	}

	debug("after solving:\n");
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n + 1; ++j) {
			debug("%.2f ", M[i][j]);
		}
		debug("\n");
	}
	debug("\n");

	return true;    // solving success
}

// M: expanded Matrix (n x n+1)
bool SolveLinearInPlace(double *M, int n)
{
#define MAT(i, j) (M + (i) * n_1)[j]
	int n_1 = n + 1;
	for (int k = 0; k < n; ++k) {    // n principle component
		double col_max = fabs(MAT(k, k)), fm;
		int maxi = k;
		for (int i = k + 1; i < n; ++i) {    // find the max in k-th col
			if ((fm = fabs(MAT(i, k))) > col_max) {
				col_max = fm;
				maxi = i;
			}
		}
		if (col_max < EPS) {        // singular matrix
			return false;        // solving failed
		}
		// swap k-th and maxi-th row
		for (int j = k; j < n + 1; ++j) {
			std::swap(MAT(k, j), MAT(maxi, j));
		}
		// perform reduction
		for (int i = k + 1; i < n; ++i) {
			double r = -MAT(i, k) / MAT(k, k);
			for (int j = k + 1; j < n + 1; ++j) {
				MAT(i, j) += r * MAT(k, j);
			}
		}
	}
	// regressive solving
	for (int k = n - 1; k >= 0; --k) {
		for (int i = 0; i < k; ++i) {
			MAT(i, n) += -MAT(i, k) / MAT(k, k) * MAT(k, n);
		}
		MAT(k, n) /= MAT(k, k);
	}
	return true;
#undef MAT
}


#endif //PHAROSA_LINEAR_HPP
