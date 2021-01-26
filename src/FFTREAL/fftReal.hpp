//-------------------------------------------------------------------
// FFT class for real data ---- Header
//      This class can execute FFT and IFFT
// Copyright (c) 2015 MIKAMI, Naoki,  2015/12/18
//-------------------------------------------------------------------

#ifndef FFT_REAL_HPP
#define FFT_REAL_HPP

#include "mbed.h"
#include <complex>  // requisite for complex

namespace Mikami
{
    typedef complex<float> Complex; // define "Complex"

    class FftReal
    {
    public:
        // Constructor
        explicit FftReal(int16_t n);
        // Destructor
        ~FftReal();
        // Execute FFT
        void Execute(const float x[], Complex y[]);
        // Execute IFFT
        void ExecuteIfft(const Complex y[], float x[]);

    private:
        const int   N_FFT_;
        const float N_INV_;

        Complex*    wTable_;    // twiddle factor
        uint16_t*   bTable_;    // for bit reversal
        Complex*    u_;         // working area

        // Processing except for last stage
        void ExcludeLastStage();
        // Use for reordering of rit reversal in IFFT
        int Index(int n) { return (N_FFT_-bTable_[n]); }

        // disallow copy constructor and assignment operator
        FftReal(const FftReal& );
        FftReal& operator=(const FftReal& );
    };
}
#endif  // FFT_REAL_HPP
