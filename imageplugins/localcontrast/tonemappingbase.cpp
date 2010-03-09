/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * Copyright (C) 2009 by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com> 
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// C++ includes.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

// Local includes.

#include "tonemappingbase.h"

namespace DigikamLocalContrastImagesPlugin
{

ToneMappingBase::ToneMappingBase()
{
    current_process_power_value = 20.0;
    preview_zoom                = 1.0;
}

ToneMappingBase::~ToneMappingBase()
{
}

void ToneMappingBase::set_blur(int nstage, REALTYPE value)
{
    if (value < 0) value = 0;
    if (value > 10000.0) value = 10000.0;
    par.stage[nstage].blur = value;
}

void ToneMappingBase::set_power(int nstage, REALTYPE value)
{
    if (value < 0) value = 0;
    if (value > 100.0) value = 100.0;
    par.stage[nstage].power = value;
}

void ToneMappingBase::set_low_saturation(int value)
{
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    par.low_saturation = value;
}

void ToneMappingBase::set_high_saturation(int value)
{
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    par.high_saturation = value;
}

void ToneMappingBase::set_stretch_contrast(bool value)
{
    par.stretch_contrast = value;
}

void ToneMappingBase::set_function_id (int value)
{
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    par.function_id = value;
}

REALTYPE ToneMappingBase::func(REALTYPE x1, REALTYPE x2)
{
    REALTYPE result = 0.5;
    REALTYPE p;

    /*
    //test function
    if (par.function_id==1)
    {
        p=pow(0.1,fabs((x2*2.0-1.0))*current_process_power_value*0.02);
        if (x2<0.5) result=pow(x1,p);
        else result=1.0-pow(1.0-x1,p);
        return result;
    };
    //test function
    if (function_id==1)
    {
        p=current_process_power_value*0.3+1e-4;
        x2=1.0/(1.0+exp(-(x2*2.0-1.0)*p*0.5));
        REALTYPE f=1.0/(1.0+exp((1.0-(x1-x2+0.5)*2.0)*p));
        REALTYPE m0=1.0/(1.0+exp((1.0-(-x2+0.5)*2.0)*p));
        REALTYPE m1=1.0/(1.0+exp((1.0-(-x2+1.5)*2.0)*p));
        result=(f-m0)/(m1-m0);
        return result;
    };
    */

    switch (par.function_id)
    {
        case 0:  //power function
            p = (REALTYPE)(pow((double)10.0,(double)fabs((x2*2.0-1.0))*current_process_power_value*0.02));
            if (x2 >= 0.5) result = pow(x1,p);
            else result = (REALTYPE)(1.0-pow((double)1.0-x1,(double)p));
            break;
        case 1:  //linear function
            p = (REALTYPE)(1.0/(1+exp(-(x2*2.0-1.0)*current_process_power_value*0.04)));
            result = (x1 < p) ? (REALTYPE)(x1*(1.0-p)/p) : (REALTYPE)((1.0-p)+(x1-p)*p/(1.0-p));
            break;
    };

    return result;
}

void ToneMappingBase::apply_parameters(const ToneMappingParameters& inpar)
{
    par = inpar;
    set_low_saturation(par.low_saturation);
    set_high_saturation(par.high_saturation);
    set_stretch_contrast(par.stretch_contrast);
    set_function_id(par.function_id);

    for (int i=0 ; i < TONEMAPPING_MAX_STAGES ; i++)
    {
        set_power(i, par.stage[i].power);
        set_blur(i, par.stage[i].blur);
    };

    update_preprocessed_values();
}

} // namespace DigikamNoiseReductionImagesPlugin
