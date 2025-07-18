// Based on the general purpose FFT routines from Ooura's Mathematical Software Packages
// (copyright Takuya OOURA, 1996-2001).
// https://www.kurims.kyoto-u.ac.jp/~ooura/fft.html

#include <math.h>

typedef float real;

// double to float conversion.
#pragma warning(disable: 4244)

namespace Vortex {

/* Fast Fourier/Cosine/Sine Transform

	dimension   :one
	data length :power of 2
	decimation  :frequency
	radix	   :8, 4, 2
	data		:inplace
	table	   :use

-------- Real DFT / Inverse of Real DFT --------

	[definition]
		<case1> RDFT
			R[k] = sum_j=0^n-1 a[j]*cos(2*pi*j*k/n), 0<=k<=n/2
			I[k] = sum_j=0^n-1 a[j]*sin(2*pi*j*k/n), 0<k<n/2
		<case2> IRDFT (excluding scale)
			a[k] = (R[0] + R[n/2]*cos(pi*k))/2 + 
			   sum_j=1^n/2-1 R[j]*cos(2*pi*j*k/n) + 
			   sum_j=1^n/2-1 I[j]*sin(2*pi*j*k/n), 0<=k<n
	[usage]
		<case1>
			ip[0] = 0; // first time only
			rdft(n, 1, a, ip, w);
		<case2>
			ip[0] = 0; // first time only
			rdft(n, -1, a, ip, w);
	[parameters]
		n			  :data length (int)
					n >= 2, n = power of 2
		a[0...n-1]	 :input/output data (real *)
					<case1>
						output data
						a[2*k] = R[k], 0<=k<n/2
						a[2*k+1] = I[k], 0<k<n/2
						a[1] = R[n/2]
					<case2>
						input data
						a[2*j] = R[j], 0<=j<n/2
						a[2*j+1] = I[j], 0<j<n/2
						a[1] = R[n/2]
		ip[0...*]	  :work area for bit reversal (int *)
					length of ip >= 2+sqrt(n/2)
					strictly, 
					length of ip >= 
						2+(1<<(int)(log(n/2+0.5)/log(2))/2).
					ip[0],ip[1] are pointers of the cos/sin table.
		w[0...n/2-1]   :cos/sin table (real *)
					w[],ip[] are initialized if ip[0] == 0.
*/

static void bitrv2(int n, int *ip, real *a)
{
	int j, j1, k, k1, l, m, m2;
	real xr, xi, yr, yi;
	
	ip[0] = 0;
	l = n;
	m = 1;
	while ((m << 3) < l) {
		l >>= 1;
		for (j = 0; j < m; j++) {
			ip[m + j] = ip[j] + l;
		}
		m <<= 1;
	}
	m2 = 2 * m;
	if ((m << 3) == l) {
		for (k = 0; k < m; k++) {
			for (j = 0; j < k; j++) {
			j1 = 2 * j + ip[k];
			k1 = 2 * k + ip[j];
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
			j1 += m2;
			k1 += 2 * m2;
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
			j1 += m2;
			k1 -= m2;
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
			j1 += m2;
			k1 += 2 * m2;
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
			}
			j1 = 2 * k + m2 + ip[k];
			k1 = j1 + m2;
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
		}
	} else {
		for (k = 1; k < m; k++) {
			for (j = 0; j < k; j++) {
			j1 = 2 * j + ip[k];
			k1 = 2 * k + ip[j];
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
			j1 += m2;
			k1 += m2;
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
			}
		}
	}
}

static void cft1st(int n, real *a, real *w)
{
	int j, k1;
	real wn4r, wtmp, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i,
		wk4r, wk4i, wk5r, wk5i, wk6r, wk6i, wk7r, wk7i;
	real x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i,
		y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i,
		y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;

	wn4r = w[2];
	x0r = a[0] + a[2];
	x0i = a[1] + a[3];
	x1r = a[0] - a[2];
	x1i = a[1] - a[3];
	x2r = a[4] + a[6];
	x2i = a[5] + a[7];
	x3r = a[4] - a[6];
	x3i = a[5] - a[7];
	y0r = x0r + x2r;
	y0i = x0i + x2i;
	y2r = x0r - x2r;
	y2i = x0i - x2i;
	y1r = x1r - x3i;
	y1i = x1i + x3r;
	y3r = x1r + x3i;
	y3i = x1i - x3r;
	x0r = a[8] + a[10];
	x0i = a[9] + a[11];
	x1r = a[8] - a[10];
	x1i = a[9] - a[11];
	x2r = a[12] + a[14];
	x2i = a[13] + a[15];
	x3r = a[12] - a[14];
	x3i = a[13] - a[15];
	y4r = x0r + x2r;
	y4i = x0i + x2i;
	y6r = x0r - x2r;
	y6i = x0i - x2i;
	x0r = x1r - x3i;
	x0i = x1i + x3r;
	x2r = x1r + x3i;
	x2i = x1i - x3r;
	y5r = wn4r * (x0r - x0i);
	y5i = wn4r * (x0r + x0i);
	y7r = wn4r * (x2r - x2i);
	y7i = wn4r * (x2r + x2i);
	a[2] = y1r + y5r;
	a[3] = y1i + y5i;
	a[10] = y1r - y5r;
	a[11] = y1i - y5i;
	a[6] = y3r - y7i;
	a[7] = y3i + y7r;
	a[14] = y3r + y7i;
	a[15] = y3i - y7r;
	a[0] = y0r + y4r;
	a[1] = y0i + y4i;
	a[8] = y0r - y4r;
	a[9] = y0i - y4i;
	a[4] = y2r - y6i;
	a[5] = y2i + y6r;
	a[12] = y2r + y6i;
	a[13] = y2i - y6r;
	if(n > 16) {
		wk1r = w[4];
		wk1i = w[5];
		x0r = a[16] + a[18];
		x0i = a[17] + a[19];
		x1r = a[16] - a[18];
		x1i = a[17] - a[19];
		x2r = a[20] + a[22];
		x2i = a[21] + a[23];
		x3r = a[20] - a[22];
		x3i = a[21] - a[23];
		y0r = x0r + x2r;
		y0i = x0i + x2i;
		y2r = x0r - x2r;
		y2i = x0i - x2i;
		y1r = x1r - x3i;
		y1i = x1i + x3r;
		y3r = x1r + x3i;
		y3i = x1i - x3r;
		x0r = a[24] + a[26];
		x0i = a[25] + a[27];
		x1r = a[24] - a[26];
		x1i = a[25] - a[27];
		x2r = a[28] + a[30];
		x2i = a[29] + a[31];
		x3r = a[28] - a[30];
		x3i = a[29] - a[31];
		y4r = x0r + x2r;
		y4i = x0i + x2i;
		y6r = x0r - x2r;
		y6i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		x2r = x1r + x3i;
		x2i = x3r - x1i;
		y5r = wk1i * x0r - wk1r * x0i;
		y5i = wk1i * x0i + wk1r * x0r;
		y7r = wk1r * x2r + wk1i * x2i;
		y7i = wk1r * x2i - wk1i * x2r;
		x0r = wk1r * y1r - wk1i * y1i;
		x0i = wk1r * y1i + wk1i * y1r;
		a[18] = x0r + y5r;
		a[19] = x0i + y5i;
		a[26] = y5i - x0i;
		a[27] = x0r - y5r;
		x0r = wk1i * y3r - wk1r * y3i;
		x0i = wk1i * y3i + wk1r * y3r;
		a[22] = x0r - y7r;
		a[23] = x0i + y7i;
		a[30] = y7i - x0i;
		a[31] = x0r + y7r;
		a[16] = y0r + y4r;
		a[17] = y0i + y4i;
		a[24] = y4i - y0i;
		a[25] = y0r - y4r;
		x0r = y2r - y6i;
		x0i = y2i + y6r;
		a[20] = wn4r * (x0r - x0i);
		a[21] = wn4r * (x0i + x0r);
		x0r = y6r - y2i;
		x0i = y2r + y6i;
		a[28] = wn4r * (x0r - x0i);
		a[29] = wn4r * (x0i + x0r);
		k1 = 4;
		for(j = 32; j < n; j += 16) {
			k1 += 4;
			wk1r = w[k1];
			wk1i = w[k1 + 1];
			wk2r = w[k1 + 2];
			wk2i = w[k1 + 3];
			wtmp = 2 * wk2i;
			wk3r = wk1r - wtmp * wk1i;
			wk3i = wtmp * wk1r - wk1i;
			wk4r = 1 - wtmp * wk2i;
			wk4i = wtmp * wk2r;
			wtmp = 2 * wk4i;
			wk5r = wk3r - wtmp * wk1i;
			wk5i = wtmp * wk1r - wk3i;
			wk6r = wk2r - wtmp * wk2i;
			wk6i = wtmp * wk2r - wk2i;
			wk7r = wk1r - wtmp * wk3i;
			wk7i = wtmp * wk3r - wk1i;
			x0r = a[j] + a[j + 2];
			x0i = a[j + 1] + a[j + 3];
			x1r = a[j] - a[j + 2];
			x1i = a[j + 1] - a[j + 3];
			x2r = a[j + 4] + a[j + 6];
			x2i = a[j + 5] + a[j + 7];
			x3r = a[j + 4] - a[j + 6];
			x3i = a[j + 5] - a[j + 7];
			y0r = x0r + x2r;
			y0i = x0i + x2i;
			y2r = x0r - x2r;
			y2i = x0i - x2i;
			y1r = x1r - x3i;
			y1i = x1i + x3r;
			y3r = x1r + x3i;
			y3i = x1i - x3r;
			x0r = a[j + 8] + a[j + 10];
			x0i = a[j + 9] + a[j + 11];
			x1r = a[j + 8] - a[j + 10];
			x1i = a[j + 9] - a[j + 11];
			x2r = a[j + 12] + a[j + 14];
			x2i = a[j + 13] + a[j + 15];
			x3r = a[j + 12] - a[j + 14];
			x3i = a[j + 13] - a[j + 15];
			y4r = x0r + x2r;
			y4i = x0i + x2i;
			y6r = x0r - x2r;
			y6i = x0i - x2i;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			x2r = x1r + x3i;
			x2i = x1i - x3r;
			y5r = wn4r * (x0r - x0i);
			y5i = wn4r * (x0r + x0i);
			y7r = wn4r * (x2r - x2i);
			y7i = wn4r * (x2r + x2i);
			x0r = y1r + y5r;
			x0i = y1i + y5i;
			a[j + 2] = wk1r * x0r - wk1i * x0i;
			a[j + 3] = wk1r * x0i + wk1i * x0r;
			x0r = y1r - y5r;
			x0i = y1i - y5i;
			a[j + 10] = wk5r * x0r - wk5i * x0i;
			a[j + 11] = wk5r * x0i + wk5i * x0r;
			x0r = y3r - y7i;
			x0i = y3i + y7r;
			a[j + 6] = wk3r * x0r - wk3i * x0i;
			a[j + 7] = wk3r * x0i + wk3i * x0r;
			x0r = y3r + y7i;
			x0i = y3i - y7r;
			a[j + 14] = wk7r * x0r - wk7i * x0i;
			a[j + 15] = wk7r * x0i + wk7i * x0r;
			a[j] = y0r + y4r;
			a[j + 1] = y0i + y4i;
			x0r = y0r - y4r;
			x0i = y0i - y4i;
			a[j + 8] = wk4r * x0r - wk4i * x0i;
			a[j + 9] = wk4r * x0i + wk4i * x0r;
			x0r = y2r - y6i;
			x0i = y2i + y6r;
			a[j + 4] = wk2r * x0r - wk2i * x0i;
			a[j + 5] = wk2r * x0i + wk2i * x0r;
			x0r = y2r + y6i;
			x0i = y2i - y6r;
			a[j + 12] = wk6r * x0r - wk6i * x0i;
			a[j + 13] = wk6r * x0i + wk6i * x0r;
		}
	}
}

static void cftmdl(int n, int l, real *a, real *w)
{
	int j, j1, j2, j3, j4, j5, j6, j7, k, k1, m;
	real wn4r, wtmp, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i,
		wk4r, wk4i, wk5r, wk5i, wk6r, wk6i, wk7r, wk7i;
	real x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i,
		y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i,
		y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;

	m = l << 3;
	wn4r = w[2];
	for(j = 0; j < l; j += 2) {
		j1 = j + l;
		j2 = j1 + l;
		j3 = j2 + l;
		j4 = j3 + l;
		j5 = j4 + l;
		j6 = j5 + l;
		j7 = j6 + l;
		x0r = a[j] + a[j1];
		x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1];
		x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3];
		x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3];
		x3i = a[j2 + 1] - a[j3 + 1];
		y0r = x0r + x2r;
		y0i = x0i + x2i;
		y2r = x0r - x2r;
		y2i = x0i - x2i;
		y1r = x1r - x3i;
		y1i = x1i + x3r;
		y3r = x1r + x3i;
		y3i = x1i - x3r;
		x0r = a[j4] + a[j5];
		x0i = a[j4 + 1] + a[j5 + 1];
		x1r = a[j4] - a[j5];
		x1i = a[j4 + 1] - a[j5 + 1];
		x2r = a[j6] + a[j7];
		x2i = a[j6 + 1] + a[j7 + 1];
		x3r = a[j6] - a[j7];
		x3i = a[j6 + 1] - a[j7 + 1];
		y4r = x0r + x2r;
		y4i = x0i + x2i;
		y6r = x0r - x2r;
		y6i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		x2r = x1r + x3i;
		x2i = x1i - x3r;
		y5r = wn4r * (x0r - x0i);
		y5i = wn4r * (x0r + x0i);
		y7r = wn4r * (x2r - x2i);
		y7i = wn4r * (x2r + x2i);
		a[j1] = y1r + y5r;
		a[j1 + 1] = y1i + y5i;
		a[j5] = y1r - y5r;
		a[j5 + 1] = y1i - y5i;
		a[j3] = y3r - y7i;
		a[j3 + 1] = y3i + y7r;
		a[j7] = y3r + y7i;
		a[j7 + 1] = y3i - y7r;
		a[j] = y0r + y4r;
		a[j + 1] = y0i + y4i;
		a[j4] = y0r - y4r;
		a[j4 + 1] = y0i - y4i;
		a[j2] = y2r - y6i;
		a[j2 + 1] = y2i + y6r;
		a[j6] = y2r + y6i;
		a[j6 + 1] = y2i - y6r;
	}
	if(m < n) {
		wk1r = w[4];
		wk1i = w[5];
		for(j = m; j < l + m; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			j4 = j3 + l;
			j5 = j4 + l;
			j6 = j5 + l;
			j7 = j6 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			y0r = x0r + x2r;
			y0i = x0i + x2i;
			y2r = x0r - x2r;
			y2i = x0i - x2i;
			y1r = x1r - x3i;
			y1i = x1i + x3r;
			y3r = x1r + x3i;
			y3i = x1i - x3r;
			x0r = a[j4] + a[j5];
			x0i = a[j4 + 1] + a[j5 + 1];
			x1r = a[j4] - a[j5];
			x1i = a[j4 + 1] - a[j5 + 1];
			x2r = a[j6] + a[j7];
			x2i = a[j6 + 1] + a[j7 + 1];
			x3r = a[j6] - a[j7];
			x3i = a[j6 + 1] - a[j7 + 1];
			y4r = x0r + x2r;
			y4i = x0i + x2i;
			y6r = x0r - x2r;
			y6i = x0i - x2i;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			x2r = x1r + x3i;
			x2i = x3r - x1i;
			y5r = wk1i * x0r - wk1r * x0i;
			y5i = wk1i * x0i + wk1r * x0r;
			y7r = wk1r * x2r + wk1i * x2i;
			y7i = wk1r * x2i - wk1i * x2r;
			x0r = wk1r * y1r - wk1i * y1i;
			x0i = wk1r * y1i + wk1i * y1r;
			a[j1] = x0r + y5r;
			a[j1 + 1] = x0i + y5i;
			a[j5] = y5i - x0i;
			a[j5 + 1] = x0r - y5r;
			x0r = wk1i * y3r - wk1r * y3i;
			x0i = wk1i * y3i + wk1r * y3r;
			a[j3] = x0r - y7r;
			a[j3 + 1] = x0i + y7i;
			a[j7] = y7i - x0i;
			a[j7 + 1] = x0r + y7r;
			a[j] = y0r + y4r;
			a[j + 1] = y0i + y4i;
			a[j4] = y4i - y0i;
			a[j4 + 1] = y0r - y4r;
			x0r = y2r - y6i;
			x0i = y2i + y6r;
			a[j2] = wn4r * (x0r - x0i);
			a[j2 + 1] = wn4r * (x0i + x0r);
			x0r = y6r - y2i;
			x0i = y2r + y6i;
			a[j6] = wn4r * (x0r - x0i);
			a[j6 + 1] = wn4r * (x0i + x0r);
		}
		k1 = 4;
		for(k = 2 * m; k < n; k += m) {
			k1 += 4;
			wk1r = w[k1];
			wk1i = w[k1 + 1];
			wk2r = w[k1 + 2];
			wk2i = w[k1 + 3];
			wtmp = 2 * wk2i;
			wk3r = wk1r - wtmp * wk1i;
			wk3i = wtmp * wk1r - wk1i;
			wk4r = 1 - wtmp * wk2i;
			wk4i = wtmp * wk2r;
			wtmp = 2 * wk4i;
			wk5r = wk3r - wtmp * wk1i;
			wk5i = wtmp * wk1r - wk3i;
			wk6r = wk2r - wtmp * wk2i;
			wk6i = wtmp * wk2r - wk2i;
			wk7r = wk1r - wtmp * wk3i;
			wk7i = wtmp * wk3r - wk1i;
			for(j = k; j < l + k; j += 2) {
				j1 = j + l;
				j2 = j1 + l;
				j3 = j2 + l;
				j4 = j3 + l;
				j5 = j4 + l;
				j6 = j5 + l;
				j7 = j6 + l;
				x0r = a[j] + a[j1];
				x0i = a[j + 1] + a[j1 + 1];
				x1r = a[j] - a[j1];
				x1i = a[j + 1] - a[j1 + 1];
				x2r = a[j2] + a[j3];
				x2i = a[j2 + 1] + a[j3 + 1];
				x3r = a[j2] - a[j3];
				x3i = a[j2 + 1] - a[j3 + 1];
				y0r = x0r + x2r;
				y0i = x0i + x2i;
				y2r = x0r - x2r;
				y2i = x0i - x2i;
				y1r = x1r - x3i;
				y1i = x1i + x3r;
				y3r = x1r + x3i;
				y3i = x1i - x3r;
				x0r = a[j4] + a[j5];
				x0i = a[j4 + 1] + a[j5 + 1];
				x1r = a[j4] - a[j5];
				x1i = a[j4 + 1] - a[j5 + 1];
				x2r = a[j6] + a[j7];
				x2i = a[j6 + 1] + a[j7 + 1];
				x3r = a[j6] - a[j7];
				x3i = a[j6 + 1] - a[j7 + 1];
				y4r = x0r + x2r;
				y4i = x0i + x2i;
				y6r = x0r - x2r;
				y6i = x0i - x2i;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				x2r = x1r + x3i;
				x2i = x1i - x3r;
				y5r = wn4r * (x0r - x0i);
				y5i = wn4r * (x0r + x0i);
				y7r = wn4r * (x2r - x2i);
				y7i = wn4r * (x2r + x2i);
				x0r = y1r + y5r;
				x0i = y1i + y5i;
				a[j1] = wk1r * x0r - wk1i * x0i;
				a[j1 + 1] = wk1r * x0i + wk1i * x0r;
				x0r = y1r - y5r;
				x0i = y1i - y5i;
				a[j5] = wk5r * x0r - wk5i * x0i;
				a[j5 + 1] = wk5r * x0i + wk5i * x0r;
				x0r = y3r - y7i;
				x0i = y3i + y7r;
				a[j3] = wk3r * x0r - wk3i * x0i;
				a[j3 + 1] = wk3r * x0i + wk3i * x0r;
				x0r = y3r + y7i;
				x0i = y3i - y7r;
				a[j7] = wk7r * x0r - wk7i * x0i;
				a[j7 + 1] = wk7r * x0i + wk7i * x0r;
				a[j] = y0r + y4r;
				a[j + 1] = y0i + y4i;
				x0r = y0r - y4r;
				x0i = y0i - y4i;
				a[j4] = wk4r * x0r - wk4i * x0i;
				a[j4 + 1] = wk4r * x0i + wk4i * x0r;
				x0r = y2r - y6i;
				x0i = y2i + y6r;
				a[j2] = wk2r * x0r - wk2i * x0i;
				a[j2 + 1] = wk2r * x0i + wk2i * x0r;
				x0r = y2r + y6i;
				x0i = y2i - y6r;
				a[j6] = wk6r * x0r - wk6i * x0i;
				a[j6 + 1] = wk6r * x0i + wk6i * x0r;
			}
		}
	}
}

static void cftfsub(int n, real *a, real *w)
{
	int j, j1, j2, j3, l;
	real x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	
	l = 2;
	if (n >= 16) {
		cft1st(n, a, w);
		l = 16;
		while ((l << 3) <= n) {
			cftmdl(n, l, a, w);
			l <<= 3;
		}
	}
	if ((l << 1) < n) {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i + x2i;
			a[j2] = x0r - x2r;
			a[j2 + 1] = x0i - x2i;
			a[j1] = x1r - x3i;
			a[j1 + 1] = x1i + x3r;
			a[j3] = x1r + x3i;
			a[j3 + 1] = x1i - x3r;
		}
	} else if ((l << 1) == n) {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			x0r = a[j] - a[j1];
			x0i = a[j + 1] - a[j1 + 1];
			a[j] += a[j1];
			a[j + 1] += a[j1 + 1];
			a[j1] = x0r;
			a[j1 + 1] = x0i;
		}
	}
}

static void cftbsub(int n, real *a, real *w)
{
	int j, j1, j2, j3, j4, j5, j6, j7, l;
	real wn4r, x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i, 
		y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i, 
		y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;
	
	l = 2;
	if (n > 16) {
		cft1st(n, a, w);
		l = 16;
		while ((l << 3) < n) {
			cftmdl(n, l, a, w);
			l <<= 3;
		}
	}
	if ((l << 2) < n) {
		wn4r = w[2];
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			j4 = j3 + l;
			j5 = j4 + l;
			j6 = j5 + l;
			j7 = j6 + l;
			x0r = a[j] + a[j1];
			x0i = -a[j + 1] - a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = -a[j + 1] + a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			y0r = x0r + x2r;
			y0i = x0i - x2i;
			y2r = x0r - x2r;
			y2i = x0i + x2i;
			y1r = x1r - x3i;
			y1i = x1i - x3r;
			y3r = x1r + x3i;
			y3i = x1i + x3r;
			x0r = a[j4] + a[j5];
			x0i = a[j4 + 1] + a[j5 + 1];
			x1r = a[j4] - a[j5];
			x1i = a[j4 + 1] - a[j5 + 1];
			x2r = a[j6] + a[j7];
			x2i = a[j6 + 1] + a[j7 + 1];
			x3r = a[j6] - a[j7];
			x3i = a[j6 + 1] - a[j7 + 1];
			y4r = x0r + x2r;
			y4i = x0i + x2i;
			y6r = x0r - x2r;
			y6i = x0i - x2i;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			x2r = x1r + x3i;
			x2i = x1i - x3r;
			y5r = wn4r * (x0r - x0i);
			y5i = wn4r * (x0r + x0i);
			y7r = wn4r * (x2r - x2i);
			y7i = wn4r * (x2r + x2i);
			a[j1] = y1r + y5r;
			a[j1 + 1] = y1i - y5i;
			a[j5] = y1r - y5r;
			a[j5 + 1] = y1i + y5i;
			a[j3] = y3r - y7i;
			a[j3 + 1] = y3i - y7r;
			a[j7] = y3r + y7i;
			a[j7 + 1] = y3i + y7r;
			a[j] = y0r + y4r;
			a[j + 1] = y0i - y4i;
			a[j4] = y0r - y4r;
			a[j4 + 1] = y0i + y4i;
			a[j2] = y2r - y6i;
			a[j2 + 1] = y2i - y6r;
			a[j6] = y2r + y6i;
			a[j6 + 1] = y2i + y6r;
		}
	} else if ((l << 2) == n) {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = -a[j + 1] - a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = -a[j + 1] + a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i - x2i;
			a[j2] = x0r - x2r;
			a[j2 + 1] = x0i + x2i;
			a[j1] = x1r - x3i;
			a[j1 + 1] = x1i - x3r;
			a[j3] = x1r + x3i;
			a[j3 + 1] = x1i + x3r;
		}
	} else {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			x0r = a[j] - a[j1];
			x0i = -a[j + 1] + a[j1 + 1];
			a[j] += a[j1];
			a[j + 1] = -a[j + 1] - a[j1 + 1];
			a[j1] = x0r;
			a[j1 + 1] = x0i;
		}
	}
}

static void rftfsub(int n, real *a, int nc, real *c)
{
	int j, k, kk, ks, m;
	real wkr, wki, xr, xi, yr, yi;
	
	m = n >> 1;
	ks = 2 * nc / m;
	kk = 0;
	for (j = 2; j < m; j += 2) {
		k = n - j;
		kk += ks;
		wkr = 0.5 - c[nc - kk];
		wki = c[kk];
		xr = a[j] - a[k];
		xi = a[j + 1] + a[k + 1];
		yr = wkr * xr - wki * xi;
		yi = wkr * xi + wki * xr;
		a[j] -= yr;
		a[j + 1] -= yi;
		a[k] += yr;
		a[k + 1] -= yi;
	}
}

static void rftbsub(int n, real *a, int nc, real *c)
{
	int j, k, kk, ks, m;
	real wkr, wki, xr, xi, yr, yi;
	
	a[1] = -a[1];
	m = n >> 1;
	ks = 2 * nc / m;
	kk = 0;
	for (j = 2; j < m; j += 2) {
		k = n - j;
		kk += ks;
		wkr = 0.5 - c[nc - kk];
		wki = c[kk];
		xr = a[j] - a[k];
		xi = a[j + 1] + a[k + 1];
		yr = wkr * xr + wki * xi;
		yi = wkr * xi - wki * xr;
		a[j] -= yr;
		a[j + 1] = yi - a[j + 1];
		a[k] += yr;
		a[k + 1] = yi - a[k + 1];
	}
	a[m + 1] = -a[m + 1];
}

static void dctsub(int n, real *a, int nc, real *c)
{
	int j, k, kk, ks, m;
	real wkr, wki, xr;
	
	m = n >> 1;
	ks = nc / n;
	kk = 0;
	for (j = 1; j < m; j++) {
		k = n - j;
		kk += ks;
		wkr = c[kk] - c[nc - kk];
		wki = c[kk] + c[nc - kk];
		xr = wki * a[j] - wkr * a[k];
		a[j] = wkr * a[j] + wki * a[k];
		a[k] = xr;
	}
	a[m] *= c[0];
}

static void dstsub(int n, real *a, int nc, real *c)
{
	int j, k, kk, ks, m;
	real wkr, wki, xr;
	
	m = n >> 1;
	ks = nc / n;
	kk = 0;
	for (j = 1; j < m; j++) {
		k = n - j;
		kk += ks;
		wkr = c[kk] - c[nc - kk];
		wki = c[kk] + c[nc - kk];
		xr = wki * a[k] - wkr * a[j];
		a[k] = wkr * a[k] + wki * a[j];
		a[j] = xr;
	}
	a[m] *= c[0];
}

static void makewt(int nw, int *ip, real *w)
{
	int j, nwh;
	real delta, x, y;

	ip[0] = nw;
	ip[1] = 1;
	if(nw > 2) {
		nwh = nw >> 1;
		delta = atan(1.0) / nwh;
		w[0] = 1;
		w[1] = 0;
		w[nwh] = cos(delta * nwh);
		w[nwh + 1] = w[nwh];
		if(nwh > 2) {
			for(j = 2; j < nwh; j += 2) {
				x = cos(delta * j);
				y = sin(delta * j);
				w[j] = x;
				w[j + 1] = y;
				w[nw - j] = y;
				w[nw - j + 1] = x;
			}
			for(j = nwh - 2; j >= 2; j -= 2) {
				x = w[2 * j];
				y = w[2 * j + 1];
				w[nwh + j] = x;
				w[nwh + j + 1] = y;
			}
			bitrv2(nw, ip + 2, w);
		}
	}
}

static void makect(int nc, int *ip, real *c)
{
	int j, nch;
	real delta;

	ip[1] = nc;
	if(nc > 1) {
		nch = nc >> 1;
		delta = atan(1.0) / nch;
		c[0] = cos(delta * nch);
		c[nch] = 0.5 * c[0];
		for(j = 1; j < nch; j++) {
			c[j] = 0.5 * cos(delta * j);
			c[nc - j] = 0.5 * sin(delta * j);
		}
	}
}

void rdft(int n, int isgn, real *a, int *ip, real *w)
{
	int nw, nc;
	real xi;

	nw = ip[0];
	if(n > (nw << 2)) {
		nw = n >> 2;
		makewt(nw, ip, w);
	}
	nc = ip[1];
	if(n > (nc << 2)) {
		nc = n >> 2;
		makect(nc, ip, w + nw);
	}
	if(isgn >= 0) {
		if(n > 4) {
			bitrv2(n, ip + 2, a);
			cftfsub(n, a, w);
			rftfsub(n, a, nc, w + nw);
		}
		else if(n == 4) {
			cftfsub(n, a, w);
		}
		xi = a[0] - a[1];
		a[0] += a[1];
		a[1] = xi;
	}
	else {
		a[1] = 0.5 * (a[0] - a[1]);
		a[0] -= a[1];
		if(n > 4) {
			rftbsub(n, a, nc, w + nw);
			bitrv2(n, ip + 2, a);
			cftbsub(n, a, w);
		}
		else if(n == 4) {
			cftfsub(n, a, w);
		}
	}
}

}; // namespace Vortex
