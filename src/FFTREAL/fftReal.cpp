//------------------------------------------------------------------------------
// FFT class for real data usind decimation-in-frequency algorithm
//      This class can execute FFT and IFFT
// Copyright (c) 2015 MIKAMI, Naoki,  2015/10/29
//------------------------------------------------------------------------------

#include "fftReal.hpp"

namespace Mikami
{
    // Constructor
    FftReal::FftReal(int16_t n)
            : N_FFT_(n), N_INV_(1.0f/n)
    {
        // __clz(): Count leading zeros
        uint32_t shifted = n << (__clz(n)+1);
        if (shifted != 0)
        {
            fprintf(stderr, "\r\nNot power of 2, in FftReal class.");
            fprintf(stderr, "\r\nForce to exit the program.");
            exit(EXIT_FAILURE); // Terminate program
        }

        wTable_ = new Complex[n/2];
        bTable_ = new uint16_t[n];
        u_ = new Complex[n];

        // calculation of twiddle factor
        Complex arg = Complex(0, -6.283185f/N_FFT_);
        for (int k=0; k<N_FFT_/2; k++)
            wTable_[k] = exp(arg*(float)k);

        // for bit reversal table
        uint16_t nShift = __clz(n) + 1;
        for (int k=0; k<n; k++)
            // __rbit(k): Reverse the bit order in a 32-bit word
            bTable_[k] = __rbit(k) >> nShift;
    }
    
    // Destructor
    FftReal::~FftReal()
    {
        delete[] wTable_;
        delete[] bTable_;
        delete[] u_;
    }

    // Execute FFT
    void FftReal::Execute(const float x[], Complex y[])
    {
        for (int n=0; n<N_FFT_; n++) u_[n] = x[n];

        // except for last stage
        ExcludeLastStage();

        // Last stage
        y[0] = u_[0] + u_[1];
        y[N_FFT_/2] = u_[0] - u_[1];
        for (int k=2; k<N_FFT_; k+=2)
            u_[k] = u_[k] + u_[k+1];

        // Reorder to bit reversal
        for (int k=1; k<N_FFT_/2; k++)
            y[k] = u_[bTable_[k]];
    }

    // Execute IFFT
    void FftReal::ExecuteIfft(const Complex y[], float x[])
    {
        int half = N_FFT_/2;

        for (int n=0; n<=half; n++) u_[n] = y[n];
        for (int n=half+1; n<N_FFT_; n++)
            u_[n] = conj(y[N_FFT_-n]);

        // except for last stage
        ExcludeLastStage();

        // Last stage including bit reversal
        x[0] = N_INV_*(u_[0].real() + u_[1].real());
        x[half] = N_INV_*(u_[0].real() - u_[1].real());

        for (int n=2; n<N_FFT_; n+=2)
        {
            float un  = u_[n].real();
            float un1 = u_[n+1].real();
            x[Index(n)]   = N_INV_*(un + un1);
            x[Index(n+1)] = N_INV_*(un - un1);
        }
    }

    // Processing except for last stage
    void FftReal::ExcludeLastStage()
    {
        uint16_t nHalf = N_FFT_/2;
        for (int stg=1; stg<N_FFT_/2; stg*=2)
        {
            uint16_t nHalf2 = nHalf*2;
            for (int kp=0; kp<N_FFT_; kp+=nHalf2)
            {
                uint16_t kx = 0;
                for (int k=kp; k<kp+nHalf; k++)
                {
                    // Butterfly operation
                    Complex uTmp = u_[k+nHalf];
                    u_[k+nHalf] = (u_[k] - uTmp)*wTable_[kx];
                    u_[k] = u_[k] + uTmp;
                    kx = kx + stg;
                }
            }
            nHalf = nHalf/2;
        }
    }
}

//Fadi and Isma 4ever//