#include "../inst/include/utility.h"

// using namespace arma;
// using namespace Rcpp;

// [[Rcpp::export]]
List runiregGibbs(arma::vec const &y, arma::mat const &X, arma::vec const &betabar, arma::mat const &A, double nu, double ssq,
                  double sigmasq, size_t R, size_t keep, size_t nprint)
{

    // Keunwoo Kim 09/09/2014

    // Purpose: perform iid draws from posterior of regression model using conjugate prior

    // Arguments:
    //  y,X
    //  betabar,A      prior mean, prior precision
    //  nu, ssq        prior on sigmasq
    //  R number of draws
    //  keep thinning parameter

    // Output: list of beta, sigmasq

    // Model:
    //  y = Xbeta + e  e ~N(0,sigmasq)
    //  y is n x 1
    //  X is n x k
    //  beta is k x 1 vector of coefficients

    // Prior:
    //  beta ~ N(betabar,sigmasq*A^-1)
    //  sigmasq ~ (nu*ssq)/chisq_nu
    //
    size_t mkeep;
    double s;
    mat RA, W, IR;
    vec z, btilde, beta;

    size_t nvar = X.n_cols;
    size_t nobs = y.size();

    vec sigmasqdraw(R / keep);
    mat betadraw(R / keep, nvar);

    mat XpX = trans(X) * X;
    vec Xpy = trans(X) * y;

    vec Abetabar = A * betabar;

    for (size_t rep = 0; rep < R; rep++)
    {

        //first draw beta | sigmasq
        IR = solve(trimatu(chol(XpX / sigmasq + A)), eye(nvar, nvar)); //trimatu interprets the matrix as upper triangular and makes solve more efficient
        btilde = (IR * trans(IR)) * (Xpy / sigmasq + Abetabar);
        beta = btilde + IR * vec(rnorm(nvar));

        //now draw sigmasq | beta
        s = sum(square(y - X * beta));
        sigmasq = (nu * ssq + s) / rchisq(1, nu + nobs)[0]; //rchisq returns a vectorized object, so using [0] allows for the conversion to double

        if ((rep + 1) % keep == 0)
        {
            mkeep = (rep + 1) / keep;
            betadraw(mkeep - 1, span::all) = trans(beta);
            sigmasqdraw[mkeep - 1] = sigmasq;
        }
    }

    return List::create(
        Named("betadraw") = betadraw,
        Named("sigmasqdraw") = NumericVector(sigmasqdraw.begin(), sigmasqdraw.end()));
}
