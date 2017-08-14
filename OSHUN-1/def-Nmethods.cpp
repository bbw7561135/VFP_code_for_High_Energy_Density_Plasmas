///////////////////////////////////////////////////////////
//   Contributing authors :	Michail Tzoufras
//
//	Created:	July 28th 2011
///////////////////////////////////////////////////////////

//   
//   This cpp file contains the definitions for the functions
//   required for the numerical methods 
///////////////////////////////////////////////////////////
//
// 
//   class Export_Formatted_Data::
//
//   This class receives the output matrices and saves the 
//   data in txt files with recognizable name after it attaches
//   a small header with information necessary for plotting. 
//
// 
//   class Restart_Facility::
//
//   This class writes restart files from each node, and 
//   reads restart files for each node.  
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//  Standard libraries
    #include <iostream>
    #include <vector>
    #include <valarray>
    #include <complex>
    #include <math.h>
    #include <stdio.h>
    #include <float.h>

//  My libraries
    #include "matrices.h"
    #include "decl-nmethods.h"



//*******************************************************************
//-------------------------------------------------------------------
     bool Gauss_Seidel(Matrix2D<double>& A, 
                       valarray< complex<double> >& b,
                       valarray< complex<double> >& xk) {
//-------------------------------------------------------------------
//   Fills solution into xk. The other matrices are not modified
//   The function returns "false" if the matrix A is not diagonally
//   dominant
//-------------------------------------------------------------------

        double     tol(1.0e-1);    // Tolerance for absolute error
        int        MAXiter(5);    // Maximum iteration allowed


//      The Matrices all have the right dimensions
//      -------------------------------------------------------------
        if ( ( A.dim1() != A.dim2()  ) || 
             ( A.dim1() != b.size()  ) ||
             ( A.dim1() != xk.size() )    )  {
            cout << "Error: The Matrices don't have the right dimensions!\n";
            exit(1);
        }
//      -------------------------------------------------------------


//      Check if the matrix A is diagonally dominant
//      -------------------------------------------------------------
        for (int i(0); i < A.dim1(); ++i){
            double rowi(0.0);
            for (int j(0); j < A.dim2(); ++j){
                rowi += A(i,j);
            }
            if (!(rowi < 2.0*A(i,i))) return false;
        }
//      -------------------------------------------------------------


//      Calculate and invert the diagonal elements once and for all
        valarray<double> invDIAG(A.dim1());
        for (int i(0); i < invDIAG.size(); ++i) invDIAG[i] = 1.0 / A(i,i);

        valarray< complex<double> > xold(xk);
        int iteration(0);         // used to count iterations
        int conv(0);              // used to test convergence

//      Start the iteration loop
        while ( (iteration++ < MAXiter) && (conv < b.size()) ) {

            xold = xk;
            for (int i(0); i < A.dim1(); ++i){
                complex<double> sigma(0.0);    // Temporary sum
                for (int j(0); j < i; ++j){
                    sigma += A(i,j)*xk[j];   
                }
                for (int j(i+1); j < A.dim2(); ++j){
                    sigma += A(i,j)*xk[j];   
                }
                xk[i] = invDIAG[i] * (b[i] - sigma);
            }

            // Calculate Dx = x_old - x_new
            xold -= xk;

//          If the relative error < prescribed tolerance everywhere the method has converged
//          |Dx(i)| < t*|x(i)| + eps 
            conv = 0;
            while ( ( conv < b.size() ) && 
                    ( abs(xold[conv]) < (tol*abs(xk[conv] + 20.0*DBL_MIN)) ) ){ 
                ++conv;
            } 

            //----> Output for testing
            //--------------------------------
            //cout << "iteration = " << iteration << "    ";
            //for (int i(0); i < b.size(); ++i){
            //    cout << xk[i] << "        ";
            //}
            //cout << "\n";
            //--------------------------------

        }

        //----> Output for testing
        //--------------------------------
        // cout << "Iterations = " << iteration-1  <<"\n";
        //for (int i(0); i < b.size(); ++i) {
        //    cout << "Error |Dx| = " << abs(xold[i]) 
        //         << ",    " 
        //         << "Tolerance * |x| = " << tol*abs(xk[i]) <<"\n";
        //}
        //--------------------------------

        return true;
    }
//-------------------------------------------------------------------
//*******************************************************************

//*******************************************************************
//-------------------------------------------------------------------
     void TridiagonalSolve (const valarray<double>& a, 
                            const valarray<double>& b, 
                                  valarray<double>& c,      
                                  valarray< complex<double> >  d,
                                  valarray< complex<double> >& x) {
//-------------------------------------------------------------------
//   Fills solution into x. Warning: will modify c and d! 
//-------------------------------------------------------------------
        size_t n(x.size());
	// Modify the coefficients. 
	c[0] /= b[0];                            // Division by zero risk. 
	d[0] /= b[0];                            // Division by zero would imply a singular matrix. 
	for (int i(1); i < n; ++i){
            double id(1.0/(b[i]-c[i-1]*a[i]));   // Division by zero risk. 
	    c[i] *= id;	                         // Last value calculated is redundant.
	    d[i] -= d[i-1] * a[i];
	    d[i] *= id;                          // d[i] = (d[i] - d[i-1] * a[i]) * id 
	}
 
	// Now back substitute. 
	x[n-1] = d[n-1];
	for (int i(n-2); i > -1; --i){
	    x[i]  = d[i];
            x[i] -= c[i] * x[i+1];               // x[i] = d[i] - c[i] * x[i + 1];
        }
    }
//-------------------------------------------------------------------


//-------------------------------------------------------------------
     bool Thomas_Tridiagonal(Matrix2D<double>& A, 
                       valarray< complex<double> >& d,
                       valarray< complex<double> >& xk) {
//-------------------------------------------------------------------
//   Fills solution into xk. The other matrices are not modified
//   The function returns "false" if the matrix A is not diagonally
//   dominant
//-------------------------------------------------------------------

//      The Matrices all have the right dimensions
//      -------------------------------------------------------------
        if ( ( A.dim1() != A.dim2()  ) || 
             ( A.dim1() != d.size()  ) ||
             ( A.dim1() != xk.size() )    )  {
            cout << "Error: The Matrices don't have the right dimensions!\n";
            exit(1);
        }
//      -------------------------------------------------------------

        valarray<double> a(d.size()), b(d.size()), c(d.size());

        for (int i(0); i < A.dim1()-1; ++i){
           a[i+1] = A(i+1,i);
        }
        for (int i(0); i < A.dim1(); ++i){
           b[i] = A(i,i);
        }
        for (int i(0); i < A.dim1()-1; ++i){
           c[i] = A(i,i+1);
        }
     
//        valarray< complex<double> > dcopy(d);
        TridiagonalSolve(a,b,c,d,xk);

        return true;
    }

//-------------------------------------------------------------------
//*******************************************************************


//*******************************************************************
//-------------------------------------------------------------------
    complex<double> Det33(/*const valarray< complex<double> >& D, */
                          Matrix2D< complex<double> >& A) {           // Determinant for a 3*3 system
//-------------------------------------------------------------------
        return A(0,0) * ( A(1,1)*A(2,2) - A(2,1)*A(1,2) ) -
               A(1,0) * ( A(0,1)*A(2,2) - A(2,1)*A(0,2) ) +
               A(2,0) * ( A(0,1)*A(1,2) - A(1,1)*A(0,2) );
    }
//-------------------------------------------------------------------

//-------------------------------------------------------------------
    complex<double> Detx33(valarray< complex<double> >& D, 
                           Matrix2D< complex<double> >& A) {         // Determinant x for a 3*3 system
//-------------------------------------------------------------------
        return D[0] * ( A(1,1)*A(2,2) - A(2,1)*A(1,2) ) -
               D[1] * ( A(0,1)*A(2,2) - A(2,1)*A(0,2) ) +
               D[2] * ( A(0,1)*A(1,2) - A(1,1)*A(0,2) );
    }
//-------------------------------------------------------------------

//-------------------------------------------------------------------
    complex<double> Dety33(valarray< complex<double> >& D, 
                           Matrix2D< complex<double> >& A) {         // Determinant y for a 3*3 system
//-------------------------------------------------------------------
        return A(0,0) * ( D[1]*A(2,2) - D[2]*A(1,2) ) -
               A(1,0) * ( D[0]*A(2,2) - D[2]*A(0,2) ) +
               A(2,0) * ( D[0]*A(1,2) - D[1]*A(0,2) );
    }
//-------------------------------------------------------------------

//-------------------------------------------------------------------
    complex<double> Detz33(valarray< complex<double> >& D,
                           Matrix2D< complex<double> >& A) {         // Determinant z for a 3*3 system
//-------------------------------------------------------------------
        return A(0,0) * ( A(1,1)*D[2] - A(2,1)*D[1] ) -
               A(1,0) * ( A(0,1)*D[2] - A(2,1)*D[0] ) +
               A(2,0) * ( A(0,1)*D[1] - A(1,1)*D[0] );
    }
//-------------------------------------------------------------------
//*******************************************************************

