/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary@daniver.demon.co.uk) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (Ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary@daniver.demon.co.uk).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <SDL/SDL.h>





static Uint32 colorMask = 0xF7DEF7DE;
static Uint32 lowPixelMask = 0x08210821;
static Uint32 qcolorMask = 0xE79CE79C;
static Uint32 qlowpixelMask = 0x18631863;
static Uint32 redblueMask = 0xF81F;
static Uint32 greenMask = 0x7E0;

int Init_2xSaI (Uint32 BitFormat)
{
    if (BitFormat == 565)
    {
	colorMask = 0xF7DEF7DE;
	lowPixelMask = 0x08210821;
	qcolorMask = 0xE79CE79C;
	qlowpixelMask = 0x18631863;
	redblueMask = 0xF81F;
	greenMask = 0x7E0;
    }
    else if (BitFormat == 555)
    {
	colorMask = 0x7BDE7BDE;
	lowPixelMask = 0x04210421;
	qcolorMask = 0x739C739C;
	qlowpixelMask = 0x0C630C63;
	redblueMask = 0x7C1F;
	greenMask = 0x3E0;
    }
    else
    {
	return 0;
    }


    return 1;
}

int GetResult1 (Uint32 A, Uint32 B, Uint32 C, Uint32 D, Uint32 E)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
	x += 1;
    else if (B == C)
	y += 1;
    if (A == D)
	x += 1;
    else if (B == D)
	y += 1;
    if (x <= 1)
	r += 1;
    if (y <= 1)
	r -= 1;
    return r;
}

int GetResult2 (Uint32 A, Uint32 B, Uint32 C, Uint32 D, Uint32 E)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
	x += 1;
    else if (B == C)
	y += 1;
    if (A == D)
	x += 1;
    else if (B == D)
	y += 1;
    if (x <= 1)
	r -= 1;
    if (y <= 1)
	r += 1;
    return r;
}

int GetResult (Uint32 A, Uint32 B, Uint32 C, Uint32 D)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
	x += 1;
    else if (B == C)
	y += 1;
    if (A == D)
	x += 1;
    else if (B == D)
	y += 1;
    if (x <= 1)
	r += 1;
    if (y <= 1)
	r -= 1;
    return r;
}

Uint32 INTERPOLATE (Uint32 A, Uint32 B)
{
    if (A != B)
    {
	return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
		(A & B & lowPixelMask));
    }
    else
	return A;
}

Uint32 Q_INTERPOLATE (Uint32 A, Uint32 B, Uint32 C, Uint32 D)
{
    register Uint32 x = ((A & qcolorMask) >> 2) +
	((B & qcolorMask) >> 2) +
	((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
    register Uint32 y = (A & qlowpixelMask) +
	(B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);

    y = (y >> 2) & qlowpixelMask;
    return x + y;
}

#define BLUE_MASK565 0x001F001F
#define RED_MASK565 0xF800F800
#define GREEN_MASK565 0x07E007E0

#define BLUE_MASK555 0x001F001F
#define RED_MASK555 0x7C007C00
#define GREEN_MASK555 0x03E003E0

void Super2xSaI(Uint8 *srcPtr, Uint32 srcPitch,
		  Uint8 *dstPtr, Uint32 dstPitch,
		 int width, int height)
{
	Uint16 *bP;
    Uint8  *dP;
    Uint32 inc_bP;
    Uint32 Nextline = srcPitch >> 1;

	Uint32 finish;
	inc_bP = 1;

	for (; height; height--)
	{
	    bP = (Uint16 *) srcPtr;
	    dP = (Uint8 *) dstPtr;

	    for (finish = width; finish; finish -= inc_bP)
	    {
		Uint32 color4, color5, color6;
		Uint32 color1, color2, color3;
		Uint32 colorA0, colorA1, colorA2, colorA3,
		    colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
		Uint32 product1a, product1b, product2a, product2b;

//---------------------------------------    B1 B2
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                           A1 A2

		colorB0 = *(bP - Nextline - 1);
		colorB1 = *(bP - Nextline);
		colorB2 = *(bP - Nextline + 1);
		colorB3 = *(bP - Nextline + 2);

		color4 = *(bP - 1);
		color5 = *(bP);
		color6 = *(bP + 1);
		colorS2 = *(bP + 2);

		color1 = *(bP + Nextline - 1);
		color2 = *(bP + Nextline);
		color3 = *(bP + Nextline + 1);
		colorS1 = *(bP + Nextline + 2);

		colorA0 = *(bP + Nextline + Nextline - 1);
		colorA1 = *(bP + Nextline + Nextline);
		colorA2 = *(bP + Nextline + Nextline + 1);
		colorA3 = *(bP + Nextline + Nextline + 2);

//--------------------------------------
		if (color2 == color6 && color5 != color3)
		{
		    product2b = product1b = color2;
		}
		else if (color5 == color3 && color2 != color6)
		{
		    product2b = product1b = color5;
		}
		else if (color5 == color3 && color2 == color6)
		{
		    register int r = 0;

		    r += GetResult (color6, color5, color1, colorA1);
		    r += GetResult (color6, color5, color4, colorB1);
		    r += GetResult (color6, color5, colorA2, colorS1);
		    r += GetResult (color6, color5, colorB2, colorS2);

		    if (r > 0)
			product2b = product1b = color6;
		    else if (r < 0)
			product2b = product1b = color5;
		    else
		    {
			product2b = product1b = INTERPOLATE (color5, color6);
		    }
		}
		else
		{
		    if (color6 == color3 && color3 == colorA1
			    && color2 != colorA2 && color3 != colorA0)
			product2b =
			    Q_INTERPOLATE (color3, color3, color3, color2);
		    else if (color5 == color2 && color2 == colorA2
			     && colorA1 != color3 && color2 != colorA3)
			product2b =
			    Q_INTERPOLATE (color2, color2, color2, color3);
		    else
			product2b = INTERPOLATE (color2, color3);

		    if (color6 == color3 && color6 == colorB1
			    && color5 != colorB2 && color6 != colorB0)
			product1b =
			    Q_INTERPOLATE (color6, color6, color6, color5);
		    else if (color5 == color2 && color5 == colorB2
			     && colorB1 != color6 && color5 != colorB3)
			product1b =
			    Q_INTERPOLATE (color6, color5, color5, color5);
		    else
			product1b = INTERPOLATE (color5, color6);
		}

		if (color5 == color3 && color2 != color6 && color4 == color5
			&& color5 != colorA2)
		    product2a = INTERPOLATE (color2, color5);
		else
		    if (color5 == color1 && color6 == color5
			&& color4 != color2 && color5 != colorA0)
		    product2a = INTERPOLATE (color2, color5);
		else
		    product2a = color2;

		if (color2 == color6 && color5 != color3 && color1 == color2
			&& color2 != colorB2)
		    product1a = INTERPOLATE (color2, color5);
		else
		    if (color4 == color2 && color3 == color2
			&& color1 != color5 && color2 != colorB0)
		    product1a = INTERPOLATE (color2, color5);
		else
		    product1a = color5;

		product1a = product1a | (product1b << 16);
		product2a = product2a | (product2b << 16);

		*((Uint32 *) dP) = product1a;
		*((Uint32 *) (dP + dstPitch)) = product2a;

		bP += inc_bP;
		dP += sizeof (Uint32);
	    }			// end of for ( finish= width etc..)

	    srcPtr   += srcPitch;
	    dstPtr   += dstPitch * 2;
	}			// endof: for (; height; height--)
}

void SuperEagle(Uint8 *srcPtr, Uint32 srcPitch, /* Uint8 *deltaPtr,  */
		 Uint8 *dstPtr, Uint32 dstPitch, int width, int height)
{
    Uint8  *dP;
    Uint16 *bP;
    Uint32 inc_bP;



	Uint32 finish;
	Uint32 Nextline = srcPitch >> 1;

	inc_bP = 1;

	for (; height ; height--)
	{
	    bP = (Uint16 *) srcPtr;
	    dP = dstPtr;
	    for (finish = width; finish; finish -= inc_bP)
	    {
		Uint32 color4, color5, color6;
		Uint32 color1, color2, color3;
		Uint32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
		Uint32 product1a, product1b, product2a, product2b;
		colorB1 = *(bP - Nextline);
		colorB2 = *(bP - Nextline + 1);

		color4 = *(bP - 1);
		color5 = *(bP);
		color6 = *(bP + 1);
		colorS2 = *(bP + 2);

		color1 = *(bP + Nextline - 1);
		color2 = *(bP + Nextline);
		color3 = *(bP + Nextline + 1);
		colorS1 = *(bP + Nextline + 2);

		colorA1 = *(bP + Nextline + Nextline);
		colorA2 = *(bP + Nextline + Nextline + 1);
		// --------------------------------------
		if (color2 == color6 && color5 != color3)
		{
		    product1b = product2a = color2;
		    if ((color1 == color2) || (color6 == colorB2))
		    {
			product1a = INTERPOLATE (color2, color5);
			product1a = INTERPOLATE (color2, product1a);
//                       product1a = color2;
		    }
		    else
		    {
			product1a = INTERPOLATE (color5, color6);
		    }

		    if ((color6 == colorS2) || (color2 == colorA1))
		    {
			product2b = INTERPOLATE (color2, color3);
			product2b = INTERPOLATE (color2, product2b);
//                       product2b = color2;
		    }
		    else
		    {
			product2b = INTERPOLATE (color2, color3);
		    }
		}
		else if (color5 == color3 && color2 != color6)
		{
		    product2b = product1a = color5;

		    if ((colorB1 == color5) || (color3 == colorS1))
		    {
			product1b = INTERPOLATE (color5, color6);
			product1b = INTERPOLATE (color5, product1b);
//                       product1b = color5;
		    }
		    else
		    {
			product1b = INTERPOLATE (color5, color6);
		    }

		    if ((color3 == colorA2) || (color4 == color5))
		    {
			product2a = INTERPOLATE (color5, color2);
			product2a = INTERPOLATE (color5, product2a);
//                       product2a = color5;
		    }
		    else
		    {
			product2a = INTERPOLATE (color2, color3);
		    }

		}
		else if (color5 == color3 && color2 == color6)
		{
		    register int r = 0;

		    r += GetResult (color6, color5, color1, colorA1);
		    r += GetResult (color6, color5, color4, colorB1);
		    r += GetResult (color6, color5, colorA2, colorS1);
		    r += GetResult (color6, color5, colorB2, colorS2);

		    if (r > 0)
		    {
			product1b = product2a = color2;
			product1a = product2b = INTERPOLATE (color5, color6);
		    }
		    else if (r < 0)
		    {
			product2b = product1a = color5;
			product1b = product2a = INTERPOLATE (color5, color6);
		    }
		    else
		    {
			product2b = product1a = color5;
			product1b = product2a = color2;
		    }
		}
		else
		{
		    product2b = product1a = INTERPOLATE (color2, color6);
		    product2b =
			Q_INTERPOLATE (color3, color3, color3, product2b);
		    product1a =
			Q_INTERPOLATE (color5, color5, color5, product1a);

		    product2a = product1b = INTERPOLATE (color5, color3);
		    product2a =
			Q_INTERPOLATE (color2, color2, color2, product2a);
		    product1b =
			Q_INTERPOLATE (color6, color6, color6, product1b);

//                    product1a = color5;
//                    product1b = color6;
//                    product2a = color2;
//                    product2b = color3;
		}
		product1a = product1a | (product1b << 16);
		product2a = product2a | (product2b << 16);

		*((Uint32 *) dP) = product1a;
		*((Uint32 *) (dP + dstPitch)) = product2a;

		bP += inc_bP;
		dP += sizeof (Uint32);
	    }			// end of for ( finish= width etc..)
	    srcPtr += srcPitch;
	    dstPtr += dstPitch * 2;
	}			// endof: for (height; height; height--)
}

void _2xSaI (Uint8 *srcPtr, Uint32 srcPitch,
	     Uint8 *dstPtr, Uint32 dstPitch, int width, int height)
{
    Uint8  *dP;
    Uint16 *bP;
    Uint32 inc_bP;


	Uint32 finish;
	Uint32 Nextline = srcPitch >> 1;
	inc_bP = 1;


	for (; height; height--)
	{
	    bP = (Uint16 *) srcPtr;
	    dP = dstPtr;

	    for (finish = width; finish; finish -= inc_bP)
	    {

		register Uint32 colorA, colorB;
		Uint32 colorC, colorD,
		    colorE, colorF, colorG, colorH,
		    colorI, colorJ, colorK, colorL,

		    colorM, colorN, colorO, colorP;
		Uint32 product, product1, product2;

//---------------------------------------
// Map of the pixels:                    I|E F|J
//                                       G|A B|K
//                                       H|C D|L
//                                       M|N O|P
		colorI = *(bP - Nextline - 1);
		colorE = *(bP - Nextline);
		colorF = *(bP - Nextline + 1);
		colorJ = *(bP - Nextline + 2);

		colorG = *(bP - 1);
		colorA = *(bP);
		colorB = *(bP + 1);
		colorK = *(bP + 2);

		colorH = *(bP + Nextline - 1);
		colorC = *(bP + Nextline);
		colorD = *(bP + Nextline + 1);
		colorL = *(bP + Nextline + 2);

		colorM = *(bP + Nextline + Nextline - 1);
		colorN = *(bP + Nextline + Nextline);
		colorO = *(bP + Nextline + Nextline + 1);
		colorP = *(bP + Nextline + Nextline + 2);

		if ((colorA == colorD) && (colorB != colorC))
		{
		    if (((colorA == colorE) && (colorB == colorL)) ||
			    ((colorA == colorC) && (colorA == colorF)
			     && (colorB != colorE) && (colorB == colorJ)))
		    {
			product = colorA;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if (((colorA == colorG) && (colorC == colorO)) ||
			    ((colorA == colorB) && (colorA == colorH)
			     && (colorG != colorC) && (colorC == colorM)))
		    {
			product1 = colorA;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		    product2 = colorA;
		}
		else if ((colorB == colorC) && (colorA != colorD))
		{
		    if (((colorB == colorF) && (colorA == colorH)) ||
			    ((colorB == colorE) && (colorB == colorD)
			     && (colorA != colorF) && (colorA == colorI)))
		    {
			product = colorB;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if (((colorC == colorH) && (colorA == colorF)) ||
			    ((colorC == colorG) && (colorC == colorD)
			     && (colorA != colorH) && (colorA == colorI)))
		    {
			product1 = colorC;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		    product2 = colorB;
		}
		else if ((colorA == colorD) && (colorB == colorC))
		{
		    if (colorA == colorB)
		    {
			product = colorA;
			product1 = colorA;
			product2 = colorA;
		    }
		    else
		    {
			register int r = 0;

			product1 = INTERPOLATE (colorA, colorC);
			product = INTERPOLATE (colorA, colorB);

			r +=
			    GetResult1 (colorA, colorB, colorG, colorE,
					colorI);
			r +=
			    GetResult2 (colorB, colorA, colorK, colorF,
					colorJ);
			r +=
			    GetResult2 (colorB, colorA, colorH, colorN,
					colorM);
			r +=
			    GetResult1 (colorA, colorB, colorL, colorO,
					colorP);

			if (r > 0)
			    product2 = colorA;
			else if (r < 0)
			    product2 = colorB;
			else
			{
			    product2 =
				Q_INTERPOLATE (colorA, colorB, colorC,
					       colorD);
			}
		    }
		}
		else
		{
		    product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);

		    if ((colorA == colorC) && (colorA == colorF)
			    && (colorB != colorE) && (colorB == colorJ))
		    {
			product = colorA;
		    }
		    else
			if ((colorB == colorE) && (colorB == colorD)
			    && (colorA != colorF) && (colorA == colorI))
		    {
			product = colorB;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if ((colorA == colorB) && (colorA == colorH)
			    && (colorG != colorC) && (colorC == colorM))
		    {
			product1 = colorA;
		    }
		    else
			if ((colorC == colorG) && (colorC == colorD)
			    && (colorA != colorH) && (colorA == colorI))
		    {
			product1 = colorC;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		}

		product = colorA | (product << 16);
		product1 = product1 | (product2 << 16);
		*((Uint32 *) dP) = product;
		*((Uint32 *) (dP + dstPitch)) = product1;

		bP += inc_bP;
		dP += sizeof (Uint32);
	    }			// end of for ( finish= width etc..)

	    srcPtr += srcPitch;
	    dstPtr += dstPitch * 2;
	}			// endof: for (height; height; height--)
}


Uint32 Bilinear(Uint32 A, Uint32 B, Uint32 x)
{
    unsigned long areaA, areaB;
    unsigned long result;

    if (A == B)
	return A;

    areaB = (x >> 11) & 0x1f;	// reduce 16 bit fraction to 5 bits
    areaA = 0x20 - areaB;

    A = (A & redblueMask) | ((A & greenMask) << 16);
    B = (B & redblueMask) | ((B & greenMask) << 16);

    result = ((areaA * A) + (areaB * B)) >> 5;

    return (result & redblueMask) | ((result >> 16) & greenMask);

}

static Uint32 Bilinear4 (Uint32 A, Uint32 B, Uint32 C, Uint32 D, Uint32 x,
			 Uint32 y)
{
    unsigned long areaA, areaB, areaC, areaD;
    unsigned long result, xy;

    x = (x >> 11) & 0x1f;
    y = (y >> 11) & 0x1f;
    xy = (x * y) >> 5;

    A = (A & redblueMask) | ((A & greenMask) << 16);
    B = (B & redblueMask) | ((B & greenMask) << 16);
    C = (C & redblueMask) | ((C & greenMask) << 16);
    D = (D & redblueMask) | ((D & greenMask) << 16);

    areaA = 0x20 + xy - x - y;
    areaB = x - xy;
    areaC = y - xy;
    areaD = xy;

    result = ((areaA * A) + (areaB * B) + (areaC * C) + (areaD * D)) >> 5;

    return (result & redblueMask) | ((result >> 16) & greenMask);
}

void Scale_2xSaI (Uint8 *srcPtr, Uint32 srcPitch, /* Uint8 * deltaPtr ,*/
		  Uint8 *dstPtr, Uint32 dstPitch, 
		  Uint32 dstWidth, Uint32 dstHeight, int width, int height)
{
    Uint8  *dP;
    Uint16 *bP;

    Uint32 w;
    Uint32 h;
    Uint32 dw;
    Uint32 dh;
    Uint32 hfinish;
    Uint32 wfinish;

    Uint32 Nextline = srcPitch >> 1;

    wfinish = (width - 1) << 16;	// convert to fixed point
    dw = wfinish / (dstWidth - 1);
    hfinish = (height - 1) << 16;	// convert to fixed point
    dh = hfinish / (dstHeight - 1);

    for (h = 0; h < hfinish; h += dh)
    {
	Uint32 y1, y2;

	y1 = h & 0xffff;	// fraction part of fixed point
	bP = (Uint16 *) (srcPtr + ((h >> 16) * srcPitch));
	dP = dstPtr;
	y2 = 0x10000 - y1;

	w = 0;

	for (; w < wfinish;)
	{
	    Uint32 A, B, C, D;
	    Uint32 E, F, G, H;
	    Uint32 I, J, K, L;
	    Uint32 x1, x2, a1, f1, f2;
	    Uint32 position, product1;

	    position = w >> 16;
	    A = bP[position];	// current pixel
	    B = bP[position + 1];	// next pixel
	    C = bP[position + Nextline];
	    D = bP[position + Nextline + 1];
	    E = bP[position - Nextline];
	    F = bP[position - Nextline + 1];
	    G = bP[position - 1];
	    H = bP[position + Nextline - 1];
	    I = bP[position + 2];
	    J = bP[position + Nextline + 2];
	    K = bP[position + Nextline + Nextline];
	    L = bP[position + Nextline + Nextline + 1];

	    x1 = w & 0xffff;	// fraction part of fixed point
	    x2 = 0x10000 - x1;

	    /*0*/ 
	    if (A == B && C == D && A == C)
		product1 = A;
	    else
	    /*1*/ 
	    if (A == D && B != C)
	    {
		f1 = (x1 >> 1) + (0x10000 >> 2);
		f2 = (y1 >> 1) + (0x10000 >> 2);
		if (y1 <= f1 && A == J && A != E)	// close to B
		{
		    a1 = f1 - y1;
		    product1 = Bilinear (A, B, a1);
		}
		else if (y1 >= f1 && A == G && A != L)	// close to C
		{
		    a1 = y1 - f1;
		    product1 = Bilinear (A, C, a1);
		}
		else if (x1 >= f2 && A == E && A != J)	// close to B
		{
		    a1 = x1 - f2;
		    product1 = Bilinear (A, B, a1);
		}
		else if (x1 <= f2 && A == L && A != G)	// close to C
		{
		    a1 = f2 - x1;
		    product1 = Bilinear (A, C, a1);
		}
		else if (y1 >= x1)	// close to C
		{
		    a1 = y1 - x1;
		    product1 = Bilinear (A, C, a1);
		}
		else if (y1 <= x1)	// close to B
		{
		    a1 = x1 - y1;
		    product1 = Bilinear (A, B, a1);
		}
	    }
	    else
	    /*2*/ 
	    if (B == C && A != D)
	    {
		f1 = (x1 >> 1) + (0x10000 >> 2);
		f2 = (y1 >> 1) + (0x10000 >> 2);
		if (y2 >= f1 && B == H && B != F)	// close to A
		{
		    a1 = y2 - f1;
		    product1 = Bilinear (B, A, a1);
		}
		else if (y2 <= f1 && B == I && B != K)	// close to D
		{
		    a1 = f1 - y2;
		    product1 = Bilinear (B, D, a1);
		}
		else if (x2 >= f2 && B == F && B != H)	// close to A
		{
		    a1 = x2 - f2;
		    product1 = Bilinear (B, A, a1);
		}
		else if (x2 <= f2 && B == K && B != I)	// close to D
		{
		    a1 = f2 - x2;
		    product1 = Bilinear (B, D, a1);
		}
		else if (y2 >= x1)	// close to A
		{
		    a1 = y2 - x1;
		    product1 = Bilinear (B, A, a1);
		}
		else if (y2 <= x1)	// close to D
		{
		    a1 = x1 - y2;
		    product1 = Bilinear (B, D, a1);
		}
	    }
	    /*3*/
	    else
	    {
		product1 = Bilinear4 (A, B, C, D, x1, y1);
	    }

//end First Pixel
	    *(Uint32 *) dP = product1;
	    dP += 2;
	    w += dw;
	}
	dstPtr += dstPitch;
    }
}
