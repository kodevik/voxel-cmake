#pragma once
#include <math.h>
#define PI 3.14159265358979

//5th order smoothstep
float smoothstep(float x) {
    return ((6.0 * x - 15.0) * x + 10) * x * x * x;
}

/*Generate square of interpolated gradients at angles given in sinCosA and 
normalised by norm. Sine and cosine of angles and normalisation term are 
predetermined for the sake of efficiency, due to cost of sin, cos and sqrt 
operations.
*/
float gSqPixel(int x, int y, int size, float sinCosA[8], float norm) {
	float v[4] = {};
	for (int k = 0; k < 4; k++) {
		int cx = x - (k % 2) * size;
		int cy = y - (k < 2) * size;
		cx = cx < 0 ? -cx : cx;
		cy = cy < 0 ? -cy : cy;
		v[k] = (cx * sinCosA[k+4] + cy * sinCosA[k]) / norm;
	}
	float top = v[0] + (v[1] - v[0]) * smoothstep((float)x/size);
	float bottom = v[2] + (v[3] - v[2]) * smoothstep((float)x/size);
	return top + (bottom - top) * smoothstep((float)y/size);
}

/*Gradient noise function, outputs array of size csize*n*csize*n clipped in
2D space to dimensions (outXDim, outYDim). Ranges between 0 and 1*/
void gradientNoise(float *out, int csize, int n, int outXDim, int outYDim) {
	int nangles = n*n + 2*n+1; //grid of gradients is size (n+1)^2
	float *sinA = new float[nangles];
	float *cosA = new float[nangles];
    float norm = 2.0 * (float)csize / sqrt(2); //normalisation term

	for (int i = 0; i < nangles; i++) {
		float angle = PI * (float)(rand() % 200) / 100;
		sinA[i] = sin(angle);
		cosA[i] = cos(angle); //fill up grid of gradient angles
	}

	int imSize = csize*n*csize*n;
	int pos = 0; //position in unclipped image
	int outPos = 0; //position in output array
	while (pos < imSize) {
		int x = pos % (csize * n);
		int y = pos / (csize * n); //convert 1D position to 2D
		if (y >= outYDim)
			break; //exit if past specified y bounds
		if (x >= outXDim) {
			pos += csize * n - outXDim; //skip to next line if past specified x bounds
			continue;
		}
		int gridX = x / csize;
		int gridY = y / csize; //angle grid coords
		//get angles for 4 corners of current grid coord
		float angles[8] = {};
		for (int j = 0; j < 4; j++) {
			angles[j] = sinA[(gridY + j / 2) * n + (gridX + j % 2)];
			angles[j+4] = cosA[(gridY + j / 2) * n + (gridX + j % 2)];
		}
		out[outPos] = (gSqPixel(x % csize, y % csize, csize, angles, norm) + 1) / 2;
		pos++;
		outPos++;
	}
	delete[] sinA;
	delete[] cosA;
}

/*Fractal noise using above gradient noise. Octaves determines # of layers, lacunarity
determines the rate at which cell size reduces per octave, and persistence determines
the influence of each successive octave of noise.
*/
void fractalNoise(float *out, int csize, int n, int octaves, float lacunarity, float persistence) {
	gradientNoise(out, csize, n, csize*n, csize*n); //initial noise layer
	float *octave = new float[csize*n*csize*n]; //noise layer container
    float norm = 1;
	for (int i = 0; i < octaves - 1; i++) {
		int newCellSize = (int)round((float)csize / pow(lacunarity, (i + 1)));
        int newN = (csize * n) / newCellSize + 1;
		gradientNoise(octave, newCellSize, newN, csize*n, csize*n);
        norm += pow(persistence, i + 1);
		for (int j = 0; j < csize*n*csize*n; j++) {
			out[j] += octave[j] * pow(persistence, i + 1);
		}
	}
    for (int j = 0; j < csize*n*csize*n; j++) {
        out[j] /= norm;
    }
	delete[] octave;
}