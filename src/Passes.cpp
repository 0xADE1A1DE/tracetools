// Copyright 2021 University of Adelaide
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "Passes.h"
#include "Util.h"
#include "WorkContext.h"
variant_t moving_avg_wind(0);

void pwpass_update_properties()
{
    moving_avg_wind = wc_gopts_get("moving_avg_wind", moving_avg_wind);
}
void pwpass_moving_average(trace_t *trace)
{
    int windsamples = moving_avg_wind.get_int();
    if (moving_avg_wind.get_int() == 0)
        return ;
    for (auto i=0u;i<trace->_traceinfo.nsamples - windsamples;i++)
    {
        double windsum = 0;
        for (auto j=i;j<i+windsamples;j++)
        {
            windsum += trace->_samples[j];
        }
        windsum /= windsamples;
        trace->_samples[i] = windsum;
    }

}

void pwpass_add_padding(const double* insamples, uint32_t nin, double* outsamples, 
        uint32_t nout, uint16_t pad_amount, double pad_value)
{
    PWASSERT(nout == 2*pad_amount + nin, "padding amount incompatible with traces");
    for (uint32_t i=0;i<pad_amount;i++)
    {
        outsamples[i] = pad_value;
    }
    memcpy(&outsamples[pad_amount], insamples, nin*sizeof(double));
    for (uint32_t i=nout-pad_amount;i<nout;i++)
    {
        outsamples[i] = pad_value;
    }
}

void pwpass_compress(const double* insamples, uint32_t nin, double* outsamples,
        uint32_t nout, uint32_t window, uint32_t overlap)
{

    uint32_t moveby=window-overlap;
    //PWASSERT(nout == nin / moveby, "incompatible trace size");
    uint32_t i=0,k=0;
    while ((i < nin -window) && (k < nout))
    {
        double sum =0;
        for (auto j=0u;j<window;j++)
        {
            sum += insamples[i+j];
        }
        outsamples[k] = sum/window;
        i += moveby;
        k += 1;
    }
    while (k< nout)
    {
        outsamples[k] = 0;
        k += 1;
    }
}
