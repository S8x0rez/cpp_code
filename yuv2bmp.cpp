#define _CRT_SECURE_NO_WARNINGS  //旧関数のwarning出力無視

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <filesystem>

typedef	unsigned char	uchar;
namespace fs = std::filesystem;

#define INPUT_BUF_MAX 		128
#define PI 3.14

typedef struct IMAGE_DATA {
	int		width;		/* 横のサイズ			*/
	int		height;		/* 縦のサイズ			*/
	int		pixel;		/* 画素数			*/
	int		color_format;		/* 420 or 444	*/
	uchar* Y;		/* 輝度データのポインタ		*/
	uchar* U;		/* 色差Uデータのポインタ	*/
	uchar* V;		/* 色差Vデータのポインタ	*/
} IMG_COMMON;

typedef struct IMAGE_HSV {
	int		width;		/* 横のサイズ			*/
	int		height;		/* 縦のサイズ			*/
	int		pixel;		/* 画素数			*/
	int		color_format;		/* 420 or 444	*/
	int* H;
	uchar* S;
	uchar* V;
} IMG_HSV;

// RGB画像データ構造体宣言 --------------------------------
typedef struct IMAGE_DATA2 {
	int		width;		/* 横のサイズ			*/
	int		height;		/* 縦のサイズ			*/
	int		pixel;		/* 画素数			*/
	uchar* R;		/* Rデータのポインタ		*/
	uchar* G;		/* Gデータのポインタ	*/
	uchar* B;		/* Bデータのポインタ	*/
} IMG_RGB;

IMG_COMMON* alloc_IMG_COMMON(int	width, int	height, int color_format);
void free_IMG_COMMON(IMG_COMMON* img);
void* m_alloc(int	size);
IMG_RGB* alloc_IMG_RGB(int	width, int	height);
void free_IMG_RGB(IMG_RGB* img);

void read_image(IMG_COMMON* img_in, FILE* fp, char filename[]);
void write_image(IMG_COMMON* imgout, FILE* fp, char filename[]);
void write_bmp(IMG_COMMON* img_work, IMG_RGB* img_rgb2, FILE* outfile);
void write_bmp_direct(IMG_RGB* img_rgb, IMG_RGB* img_rgb2, FILE* outfile);
FILE* open_image(char filename[], char* mode);
void copy_image(IMG_COMMON* img1, IMG_COMMON* img2, int param);
void trans_image_420to444(IMG_COMMON* img1, IMG_COMMON* img2, int param);
void setvalue_image(IMG_COMMON* img1, int Yvalue, int Uvalue, int Vvalue);

void YUV2RGBm_image(IMG_COMMON* imgYUV, IMG_RGB* imgRGB, int color_matrix);
void RGB2YUVm_image(IMG_RGB* imgRGB, IMG_COMMON* imgYUV, int color_matrix);

//int rounding(double dv)　ダブル型値を0-255の範囲に丸める
unsigned char rounding(double dv)
{
	unsigned char ucv;
	int iv = (int)(dv + 0.5);
	if (iv < 0) {
		iv = 0;
	}
	else if (iv > 255) {
		iv = 255;
	}
	ucv = (unsigned char)iv;
	return ucv;
}
//int rounding_integer(int idv) 整数値を0-255の範囲に丸める
unsigned char rounding_integer(int idv)
{
	unsigned char ucv;
	int iv;
	iv = idv;
	if (idv < 0) {
		iv = 0;
	}
	else if (idv > 255) {
		iv = 255;
	}
	ucv = (unsigned char)iv;
	return ucv;
}

//----------------------------------------------------------------
//	Function-ID	alloc_IMG_COMMON
//	Function-Name	IMGデータ領域確保
//	Abstract	IMG_COMMONデータのメモリを確保する
//	Argument	int	width	= 画像横サイズ
//				int	height	= 画像縦サイズ
//				int	color_format	= 色フォーマット（420 or 444）
//	Return-Value	IMG_COMMON *		= 確保した領域のポインタ
//	Special_Desc	
//----------------------------------------------------------------
IMG_COMMON*
alloc_IMG_COMMON(int	width, int	height, int color_format)
{
	IMG_COMMON* img;

	img = (IMG_COMMON*)m_alloc(sizeof(IMG_COMMON));
	img->width = width;
	img->height = height;
	img->pixel = img->width * img->height;
	img->color_format = color_format;
	img->Y = (uchar*)m_alloc(sizeof(uchar) * img->pixel);
	if (color_format == 420) {
		img->U = (uchar*)m_alloc(sizeof(uchar) * img->pixel / 4);
		img->V = (uchar*)m_alloc(sizeof(uchar) * img->pixel / 4);
	}
	else {
		img->U = (uchar*)m_alloc(sizeof(uchar) * img->pixel);
		img->V = (uchar*)m_alloc(sizeof(uchar) * img->pixel);
	}

	return (img);
}

//----------------------------------------------------------------
//	Function-ID	alloc_IMG_HSV
//	Function-Name	IMGデータ領域確保
//	Abstract	IMG_HSVデータのメモリを確保する
//	Argument	int	width	= 画像横サイズ
//				int	height	= 画像縦サイズ
//				int	color_format	= 色フォーマット（420 or 444）
//	Return-Value	IMG_COMMON *		= 確保した領域のポインタ
//	Special_Desc	
//----------------------------------------------------------------
IMG_HSV*
alloc_IMG_HSV(int	width, int	height)
{
	IMG_HSV* img;

	img = (IMG_HSV*)m_alloc(sizeof(IMG_HSV));
	img->width = width;
	img->height = height;
	img->pixel = img->width * img->height;
	img->H = (int*)m_alloc(sizeof(int) * img->pixel);
	img->S = (uchar*)m_alloc(sizeof(uchar) * img->pixel);
	img->V = (uchar*)m_alloc(sizeof(uchar) * img->pixel);

	return (img);
}

//------------------------------------------------------------------------------
//	Function-ID	free_IMG_COMMON
//	Function-Name	IMG_COMMONデータメモリ解放
//	Abstract	IMG_COMMONデータの領域を解放する
//	Argument	IMG_COMMON	*img	= 画像構造体
//	Return-Value	Nothing
//	Special_Desc	
//------------------------------------------------------------------------------
void
free_IMG_COMMON(IMG_COMMON* img)
{
	if (img->Y)		free(img->Y);
	if (img->U)		free(img->U);
	if (img->V)		free(img->V);
	free(img);
}

//------------------------------------------------------------------------------
//	Function-ID	free_IMG_HSV
//	Function-Name	IMG_HSVデータメモリ解放
//	Abstract	IMG_HSVデータの領域を解放する
//	Argument	IMG_HSV	*img	= 画像構造体
//	Return-Value	Nothing
//	Special_Desc	
//------------------------------------------------------------------------------
void
free_IMG_HSV(IMG_HSV* img)
{
	if (img->H)		free(img->H);
	if (img->S)		free(img->S);
	if (img->V)		free(img->V);
	free(img);
}

//----------------------------------------------------------------
//	Function-ID	alloc_IMG_RGB
//	Function-Name	IMGデータ領域確保
//	Abstract	IMG_RGBデータのメモリを確保する
//	Argument	int	width	= 画像横サイズ
//				int	height	= 画像縦サイズ
//	Return-Value	IMG_RGB *		= 確保した領域のポインタ
//	Special_Desc	
//----------------------------------------------------------------
IMG_RGB*
alloc_IMG_RGB(int	width, int	height)
{
	IMG_RGB* img;

	img = (IMG_RGB*)m_alloc(sizeof(IMG_RGB));
	img->width = width;
	img->height = height;
	img->pixel = img->width * img->height;
	img->R = (uchar*)m_alloc(sizeof(uchar) * img->pixel);
	img->G = (uchar*)m_alloc(sizeof(uchar) * img->pixel);
	img->B = (uchar*)m_alloc(sizeof(uchar) * img->pixel);

	return (img);
}

//------------------------------------------------------------------------------
//	Function-ID	free_IMG_RGB
//	Function-Name	IMG_RGBデータメモリ解放
//	Abstract	IMG_RGBデータの領域を解放する
//	Argument	IMG_RGB	*img	= 画像構造体
//	Return-Value	Nothing
//	Special_Desc	
//------------------------------------------------------------------------------
void
free_IMG_RGB(IMG_RGB* img)
{
	if (img->R)		free(img->R);
	if (img->G)		free(img->G);
	if (img->B)		free(img->B);
	free(img);
}

//------------------------------------------------------------------------------
//	Function-ID	m_alloc
//	Function-Name	メモリ確保
//	Abstract	異常終了時にエラーメッセージを返すmalloc関数
//	Argument	int	size	= 確保するメモリサイズ
//	Return-Value	void	*a	= 確保したメモリのアドレス
//	Special_Desc
//------------------------------------------------------------------------------
void*
m_alloc(int	size)
{
	void* a;

	if ((a = malloc(size)) == NULL) {
		fprintf(stderr, "m_alloc: memory allocation error.\n");
		exit(-1);
	}

	return (a);
}

//----------------------------------------------------------------
//	Function-ID	open_image
//	Function-Name	
//	Abstract	画像データファイルをオープンする
//
//	Argument    char filename : 読み込みファイル名
//              char mode : オープンモード( "rb", or "wb"）
//			      "rb"：読み込み，　"wb"：書き出し
//
//	Return-Value	FILE fp:ファイルポインタ
//	Special_Desc	
//----------------------------------------------------------------
FILE* open_image(char filename[], const char* mode)
{
	FILE* fp;
	if ((fp = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "File open error = %s\n", filename);
		exit(-1);
	}
	return(fp);
}

//----------------------------------------------------------------
//	Function-ID	read_image
//	Function-Name	
//	Abstract	画像データをIMG_COMMON型構造体としてファイルから読み込む
//	               IMG_COMMONはグローバルで以下の形で宣言されていること	
//
//                  typedef struct IMAGE_DATA {
//                       int		width;		/* 横のサイズ
//                       int		height;		/* 縦のサイズ
//                       int		pixel;		/* 画素数
//						 int		color_format;		/* 420 or 444	*/
//                       uchar	*Y;		/* 輝度データのポインタ
//                       uchar	*U;		/* 色差Uデータのポインタ
//                       uchar	*V;		/* 色差Vデータのポインタ
//                  } IMG_COMMON;
//
//	Argument	IMG_COMMON *img_in: 読み込まれる画像データが入る
//		        FILE *fp: 読み込み先ポインタ
//		        chara filename : 読み込みファイル名
//			
//	Return-Value	
//	Special_Desc	
//----------------------------------------------------------------
void  read_image(IMG_COMMON* img_in, FILE* fp, char filename[])
{

	int width, height;
	int size, sizec;

	width = img_in->width;
	height = img_in->height;
	size = width * height;
	if (img_in->color_format == 420) {
		sizec = size / 4;
	}
	else {
		sizec = size;
	}

	if (fread(img_in->Y, sizeof(uchar), size, fp) != size) {	/* Y信号 */
		fprintf(stderr, "File read error = %s(Y sig)\n", filename);
		exit(-1);
	}
	if (fread(img_in->U, sizeof(uchar), sizec, fp) != sizec) {	/* U信号 */
		fprintf(stderr, "File read error = %s(U sig)\n", filename);
		exit(-1);
	}

	if (fread(img_in->V, sizeof(uchar), sizec, fp) != sizec) {	/* V信号 */
		fprintf(stderr, "File read error = %s(V sig)\n", filename);
		exit(-1);
	}

}

//----------------------------------------------------------------
//	Function-ID	write_image
//	Function-Name	
//	Abstract	IMG_COMMON型構造体の画像データをファイルに出力する
//	               IMG_COMMONはグローバルで以下の形で宣言されていること	
//
//                  typedef struct IMAGE_DATA {
//                       int		width;		/* 横のサイズ
//                       int		height;		/* 縦のサイズ
//                       int		pixel;		/* 画素数
//						 int		color_format;		/* 420 or 444	*/
//                       uchar	*Y;		/* 輝度データのポインタ
//                       uchar	*U;		/* 色差Uデータのポインタ
//                       uchar	*V;		/* 色差Vデータのポインタ
//                  } IMG_COMMON;
//
//
//	Argument	IMG_COMMON *imgout: 書き込む画像データ
//		        FILE *fp: 書き込み先ポインタ
//		        chara filename : 書き出しファイル名
//			
//	Return-Value	
//	Special_Desc	
//----------------------------------------------------------------
void  write_image(IMG_COMMON* imgout, FILE* fp, char filename[])
{

	int width, height;
	int size, sizec;

	width = imgout->width;
	height = imgout->height;
	size = width * height;
	if (imgout->color_format == 420) {
		sizec = size / 4;
	}
	else {
		sizec = size;
	}

	if (fwrite(imgout->Y, sizeof(uchar), size, fp) != size) {	/* Y信号出力 */
		fprintf(stderr, "File write error = %s(Y sig)\n", filename);
		exit(-1);
	}
	if (fwrite(imgout->U, sizeof(uchar), sizec, fp) != sizec) { /* U信号 */
		fprintf(stderr, "File write error = %s(U sig)\n", filename);
		exit(-1);
	}

	if (fwrite(imgout->V, sizeof(uchar), sizec, fp) != sizec) { /* V信号 */
		fprintf(stderr, "File write error = %s(V sig)\n", filename);
		exit(-1);
	}


}

//----------------------------------------------------------------
//	Function-ID	write_bmp
//	Function-Name	ビットマップ書き込み
//	Abstract	YUVをRGB変換してimg_rbg2に格納
//	Argument	IMG_COMMON *img:	YUV信号が格納されている画像データ(444)
//              IMG_RGB *img_rgb:       色変換後の画像格納領域（作業用配列）   
//              FILE *outfile:       書き込み先ポインタ
//	Special_Desc	24ビットのみ対応
//
//            +---------------------------------
//               Coded by Takashi Yoshino
//               Revised by Y.Yashima 2012.09.15
//----------------------------------------------------------------
void write_bmp(IMG_COMMON* img, IMG_RGB* img_rgb, FILE* outfile) {
	int width, height;
	int size;

	width = img->width;
	height = img->height;
	size = width * height;
	int k, j;
	double r, g, b;

	//YUV-RGB変換と並べ替え
	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			/* BT.709 */
			r = img->Y[j + k * width] + 1.5748 * (img->V[j + k * width] - 128);
			g = img->Y[j + k * width] - 0.1873 * (img->U[j + k * width] - 128) - 0.4681 * (img->V[j + k * width] - 128);
			b = img->Y[j + k * width] + 1.8556 * (img->U[j + k * width] - 128);
			/* BT.601 */
//			r = img->Y[j + k*width] + 1.402 * (img->V[j + k*width] - 128);
//			g = img->Y[j + k*width] - 0.344136 * (img->U[j + k*width] - 128) - 0.714136 * (img->V[j + k*width] - 128);
//			b = img->Y[j + k*width] + 1.772 * (img->U[j + k*width] - 128);
			img_rgb->R[j + (height - k - 1) * width] = rounding(r);
			img_rgb->G[j + (height - k - 1) * width] = rounding(g);
			img_rgb->B[j + (height - k - 1) * width] = rounding(b);
		}
	}

	//ヘッダ書き込み
	uchar header_buf[54];
	unsigned int file_size;
	unsigned int offset_to_data;
	unsigned long info_header_size;
	unsigned int planes;
	unsigned int color;
	unsigned long compress;
	unsigned long data_size;
	long xppm;
	long yppm;

	file_size = size + 54;
	offset_to_data = 54;
	info_header_size = 40;
	planes = 1;
	color = 24;
	compress = 0;
	data_size = size;
	xppm = 1;
	yppm = 1;

	header_buf[0] = 'B';
	header_buf[1] = 'M';
	memcpy(header_buf + 2, &file_size, sizeof(file_size));
	header_buf[6] = 0;
	header_buf[7] = 0;
	header_buf[8] = 0;
	header_buf[9] = 0;
	memcpy(header_buf + 10, &offset_to_data, sizeof(file_size));
	header_buf[11] = 0;
	header_buf[12] = 0;
	header_buf[13] = 0;
	memcpy(header_buf + 14, &info_header_size, sizeof(info_header_size));
	header_buf[15] = 0;
	header_buf[16] = 0;
	header_buf[17] = 0;
	memcpy(header_buf + 18, &width, sizeof(width));
	memcpy(header_buf + 22, &height, sizeof(height));
	memcpy(header_buf + 26, &planes, sizeof(planes));
	memcpy(header_buf + 28, &color, sizeof(color));
	memcpy(header_buf + 30, &compress, sizeof(compress));
	memcpy(header_buf + 34, &data_size, sizeof(data_size));
	memcpy(header_buf + 38, &xppm, sizeof(xppm));
	memcpy(header_buf + 42, &yppm, sizeof(yppm));
	header_buf[46] = 0;
	header_buf[47] = 0;
	header_buf[48] = 0;
	header_buf[49] = 0;
	header_buf[50] = 0;
	header_buf[51] = 0;
	header_buf[52] = 0;
	header_buf[53] = 0;

	fwrite(header_buf, sizeof(unsigned char), 54, outfile);

	for (int kk = 0; kk < size; kk++) {
		fputc(img_rgb->B[kk], outfile);		//Bを出力
		fputc(img_rgb->G[kk], outfile);		//G
		fputc(img_rgb->R[kk], outfile);		//R
	}
}

//----------------------------------------------------------------
//	Function-ID	write_bmp_direct
//	Function-Name	RGBを直接ビットマップ書き込み
//	Abstract	IMG_RGM型RGBデータをビットマップとして書き込み
//	Argument	
//              IMG_RGB *img_rgb:    書き込むべきRGB画像データ   
//              IMG_RGB *img_rgb2:    RGB画像データ（作業用配列）   
//              FILE *outfile:       書き込み先ポインタ
//	Special_Desc	24ビットのみ対応
//
//            +---------------------------------
//               Coded by Takashi Yoshino
//               Revised by Y.Yashima 2012.09.15
//----------------------------------------------------------------
void write_bmp_direct(IMG_RGB* img_rgb, IMG_RGB* img_rgb2, FILE* outfile)
{
	int width, height;
	int size;

	width = img_rgb->width;
	height = img_rgb->height;
	size = width * height;
	int k, j;
	double r, g, b;

	//YUV-RGB変換と並べ替え
	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			r = img_rgb->R[j + k * width];
			g = img_rgb->G[j + k * width];
			b = img_rgb->B[j + k * width];
			img_rgb2->R[j + (height - k - 1) * width] = rounding(r);
			img_rgb2->G[j + (height - k - 1) * width] = rounding(g);
			img_rgb2->B[j + (height - k - 1) * width] = rounding(b);
		}
	}

	//ヘッダ書き込み
	uchar header_buf[54];
	unsigned int file_size;
	unsigned int offset_to_data;
	unsigned long info_header_size;
	unsigned int planes;
	unsigned int color;
	unsigned long compress;
	unsigned long data_size;
	long xppm;
	long yppm;

	file_size = size + 54;
	offset_to_data = 54;
	info_header_size = 40;
	planes = 1;
	color = 24;
	compress = 0;
	data_size = size;
	xppm = 1;
	yppm = 1;

	header_buf[0] = 'B';
	header_buf[1] = 'M';
	memcpy(header_buf + 2, &file_size, sizeof(file_size));
	header_buf[6] = 0;
	header_buf[7] = 0;
	header_buf[8] = 0;
	header_buf[9] = 0;
	memcpy(header_buf + 10, &offset_to_data, sizeof(file_size));
	header_buf[11] = 0;
	header_buf[12] = 0;
	header_buf[13] = 0;
	memcpy(header_buf + 14, &info_header_size, sizeof(info_header_size));
	header_buf[15] = 0;
	header_buf[16] = 0;
	header_buf[17] = 0;
	memcpy(header_buf + 18, &width, sizeof(width));
	memcpy(header_buf + 22, &height, sizeof(height));
	memcpy(header_buf + 26, &planes, sizeof(planes));
	memcpy(header_buf + 28, &color, sizeof(color));
	memcpy(header_buf + 30, &compress, sizeof(compress));
	memcpy(header_buf + 34, &data_size, sizeof(data_size));
	memcpy(header_buf + 38, &xppm, sizeof(xppm));
	memcpy(header_buf + 42, &yppm, sizeof(yppm));
	header_buf[46] = 0;
	header_buf[47] = 0;
	header_buf[48] = 0;
	header_buf[49] = 0;
	header_buf[50] = 0;
	header_buf[51] = 0;
	header_buf[52] = 0;
	header_buf[53] = 0;

	fwrite(header_buf, sizeof(unsigned char), 54, outfile);

	for (int kk = 0; kk < size; kk++) {
		fputc(img_rgb2->B[kk], outfile);		//Bを出力
		fputc(img_rgb2->G[kk], outfile);		//G
		fputc(img_rgb2->R[kk], outfile);		//R
	}
}

//----------------------------------------------------------------
//	Function-ID	copy_image
//	Function-Name	
//	Abstract	IMG_COMMON型構造体データをコピーする（構造体ポインタを引き渡す）
//	               IMG_COMMONはグローバルで以下の形で宣言されていること	
//
//                  typedef struct IMAGE_DATA {
//                       int		width;		/* 横のサイズ
//                       int		height;		/* 縦のサイズ
//                       int		pixel;		/* 画素数
//						 int		color_format;		/* 420 or 444	*/
//                       uchar	*Y;		/* 輝度データのポインタ
//                       uchar	*U;		/* 色差Uデータのポインタ
//                       uchar	*V;		/* 色差Vデータのポインタ
//                  } IMG_COMMON;
//
//
//	Argument	IMG_COMMON *img1: copy元画像データ
//		        IMG_COMMON *img2: copy先画像データ
//		        int param : 1(0以外)のときYUVすべてコピー，0のときYだけコピーしてUVには128を入れる
//			
//	Return-Value	
//	Special_Desc	
//----------------------------------------------------------------
void  copy_image(IMG_COMMON* img1, IMG_COMMON* img2, int param)
{

	int j, k;
	int width, height;
	int widthc, heightc;

	width = img1->width;
	height = img1->height;
	if (img1->color_format == 420) {
		widthc = width / 2;
		heightc = height / 2;
	}
	else {
		widthc = width;
		heightc = height;
	}

	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			img2->Y[j + k * width] = img1->Y[j + k * width];
		}
	}

	if (param == 0) {
		for (k = 0; k < heightc; k++) {
			for (j = 0; j < widthc; j++) {
				img2->U[j + k * widthc] = 128;
				img2->V[j + k * widthc] = 128;
			}
		}
	}
	else {
		for (k = 0; k < heightc; k++) {
			for (j = 0; j < widthc; j++) {
				img2->U[j + k * widthc] = img1->U[j + k * widthc];
				img2->V[j + k * widthc] = img1->V[j + k * widthc];
			}
		}
	}

}



//----------------------------------------------------------------
//	Function-ID	trans_image_420to444
//	Function-Name	
//	Abstract	420YUVを444YUV形式に変換する（構造体ポインタ引き渡し）
//	             IMG_COMMONはグローバルで以下の形で宣言されていること	
//
//                  typedef struct IMAGE_DATA {
//                       int		width;		/* 横のサイズ
//                       int		height;		/* 縦のサイズ
//                       int		pixel;		/* 画素数
//						 int		color_format;		/* 420 or 444	*/
//                       uchar	*Y;		/* 輝度データのポインタ
//                       uchar	*U;		/* 色差Uデータのポインタ
//                       uchar	*V;		/* 色差Vデータのポインタ
//                  } IMG_COMMON;
//
//	Argument	IMG_COMMON *img1: copy元420画像データ
//		        IMG_COMMON *img2: copy先444画像データ
//		        int param : 1(0以外)のときYUVすべて変換，0のときYだけ変換（コピー）してUVには128を入れる
//			
//	Return-Value	
//	Special_Desc	
//----------------------------------------------------------------
void  trans_image_420to444(IMG_COMMON* img1, IMG_COMMON* img2, int param)
{

	int j, k;
	int width, height;
	int widthc, heightc;

	width = img1->width;
	height = img1->height;
	widthc = width / 2;
	heightc = height / 2;

	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			img2->Y[j + k * width] = img1->Y[j + k * width];
		}
	}

	if (param == 0) {
		for (k = 0; k < height; k++) {
			for (j = 0; j < width; j++) {
				img2->U[j + k * width] = 128;
				img2->V[j + k * width] = 128;
			}
		}
	}
	else {

		for (k = 0; k < heightc - 1; k++) {
			for (j = 0; j < widthc - 1; j++) {
				img2->U[2 * j + 1 + (2 * k + 1) * width] = (9 * img1->U[j + k * widthc] + 4 * img1->U[j + 1 + k * widthc] + 4 * img1->U[j + (k + 1) * widthc] + 3 * img1->U[j + 1 + (k + 1) * widthc] + 10) / 20;
				img2->U[2 * j + 2 + (2 * k + 1) * width] = (4 * img1->U[j + k * widthc] + 9 * img1->U[j + 1 + k * widthc] + 3 * img1->U[j + (k + 1) * widthc] + 4 * img1->U[j + 1 + (k + 1) * widthc] + 10) / 20;
				img2->U[2 * j + 1 + (2 * k + 2) * width] = (4 * img1->U[j + k * widthc] + 3 * img1->U[j + 1 + k * widthc] + 9 * img1->U[j + (k + 1) * widthc] + 4 * img1->U[j + 1 + (k + 1) * widthc] + 10) / 20;
				img2->U[2 * j + 2 + (2 * k + 2) * width] = (3 * img1->U[j + k * widthc] + 4 * img1->U[j + 1 + k * widthc] + 4 * img1->U[j + (k + 1) * widthc] + 9 * img1->U[j + 1 + (k + 1) * widthc] + 10) / 20; ;
				img2->V[2 * j + 1 + (2 * k + 1) * width] = (9 * img1->V[j + k * widthc] + 4 * img1->V[j + 1 + k * widthc] + 4 * img1->V[j + (k + 1) * widthc] + 3 * img1->V[j + 1 + (k + 1) * widthc] + 10) / 20;
				img2->V[2 * j + 2 + (2 * k + 1) * width] = (4 * img1->V[j + k * widthc] + 9 * img1->V[j + 1 + k * widthc] + 3 * img1->V[j + (k + 1) * widthc] + 4 * img1->V[j + 1 + (k + 1) * widthc] + 10) / 20;
				img2->V[2 * j + 1 + (2 * k + 2) * width] = (4 * img1->V[j + k * widthc] + 3 * img1->V[j + 1 + k * widthc] + 9 * img1->V[j + (k + 1) * widthc] + 4 * img1->V[j + 1 + (k + 1) * widthc] + 10) / 20;
				img2->V[2 * j + 2 + (2 * k + 2) * width] = (3 * img1->V[j + k * widthc] + 4 * img1->V[j + 1 + k * widthc] + 4 * img1->V[j + (k + 1) * widthc] + 9 * img1->V[j + 1 + (k + 1) * widthc] + 10) / 20; ;
			}
		}

		k = 0;
		for (j = 0; j < widthc - 1; j++) {
			img2->U[2 * j + 1 + k * width] = (18 * img1->U[j + k * widthc] + 8 * img1->U[j + 1 + k * widthc] + 13) / 26;
			img2->U[2 * j + 2 + k * width] = (8 * img1->U[j + k * widthc] + 18 * img1->U[j + 1 + k * widthc] + 13) / 26;
			img2->V[2 * j + 1 + k * width] = (18 * img1->V[j + k * widthc] + 8 * img1->V[j + 1 + k * widthc] + 13) / 26;
			img2->V[2 * j + 2 + k * width] = (8 * img1->V[j + k * widthc] + 18 * img1->V[j + 1 + k * widthc] + 13) / 26;
		}
		k = heightc - 1;
		for (j = 0; j < widthc - 1; j++) {
			img2->U[2 * j + 1 + (2 * k + 1) * width] = (18 * img1->U[j + k * widthc] + 8 * img1->U[j + 1 + k * widthc] + 13) / 26;
			img2->U[2 * j + 2 + (2 * k + 1) * width] = (8 * img1->U[j + k * widthc] + 18 * img1->U[j + 1 + k * widthc] + 13) / 26;
			img2->V[2 * j + 1 + (2 * k + 1) * width] = (18 * img1->V[j + k * widthc] + 8 * img1->V[j + 1 + k * widthc] + 13) / 26;
			img2->V[2 * j + 2 + (2 * k + 1) * width] = (8 * img1->V[j + k * widthc] + 18 * img1->V[j + 1 + k * widthc] + 13) / 26;
		}
		for (k = 0; k < heightc - 1; k++) {
			j = 0;
			img2->U[j + (2 * k + 1) * width] = (18 * img1->U[j + k * widthc] + 8 * img1->U[j + (k + 1) * widthc] + 13) / 26;
			img2->U[j + (2 * k + 2) * width] = (8 * img1->U[j + k * widthc] + 18 * img1->U[j + (k + 1) * widthc] + 13) / 26;
			img2->V[j + (2 * k + 1) * width] = (18 * img1->V[j + k * widthc] + 8 * img1->V[j + (k + 1) * widthc] + 13) / 26;
			img2->V[j + (2 * k + 2) * width] = (8 * img1->V[j + k * widthc] + 18 * img1->V[j + (k + 1) * widthc] + 13) / 26;
			j = widthc - 1;
			img2->U[2 * j + 1 + (2 * k + 1) * width] = (18 * img1->U[j + k * widthc] + 8 * img1->U[j + (k + 1) * widthc] + 13) / 26;
			img2->U[2 * j + 1 + (2 * k + 2) * width] = (8 * img1->U[j + k * widthc] + 18 * img1->U[j + (k + 1) * widthc] + 13) / 26;
			img2->V[2 * j + 1 + (2 * k + 1) * width] = (18 * img1->V[j + k * widthc] + 8 * img1->V[j + (k + 1) * widthc] + 13) / 26;
			img2->V[2 * j + 1 + (2 * k + 2) * width] = (8 * img1->V[j + k * widthc] + 18 * img1->V[j + (k + 1) * widthc] + 13) / 26;
		}
		k = 0; j = 0;                img2->U[j + k * width] = img1->U[j + k * widthc];
		img2->V[j + k * width] = img1->V[j + k * widthc];
		k = 0; j = widthc - 1;         img2->U[2 * j + 1 + k * width] = img1->U[j + k * widthc];
		img2->V[2 * j + 1 + k * width] = img1->V[j + k * widthc];
		k = heightc - 1; j = 0;        img2->U[j + (2 * k + 1) * width] = img1->U[j + k * widthc];
		img2->V[j + (2 * k + 1) * width] = img1->V[j + k * widthc];
		k = heightc - 1; j = widthc - 1; img2->U[2 * j + 1 + (2 * k + 1) * width] = img1->U[j + k * widthc];
		img2->V[2 * j + 1 + (2 * k + 1) * width] = img1->V[j + k * widthc];

	}

}

//----------------------------------------------------------------
//	Function-ID	YUV2RGBm_image
//	Function-Name	
//	Abstract	IMG_COMMON型構造体の画像データ(YUV444）に対し，IMG_RGB型構造体のRGBへの変換を行う
//
//	Argument	IMG_COMMON *imgYUV: 原画像データ(input)
//		        IMG_RGB *imgRGB: 処理画像データ(output)
//		        int color_matrix: 色変換マトリクスパラメータ（601 or 709) (input)
//			                       601: BT.601準拠(Y/Cb/Cr)
//                                 709: BT.709準拠(Y/Pb/Pr)
//	Return-Value	
//	
//----------------------------------------------------------------

void  YUV2RGBm_image(IMG_COMMON* imgYUV, IMG_RGB* imgRGB, int color_matrix)
{

	int j, k;
	int width, height;
	double r, g, b;

	width = imgYUV->width;
	height = imgYUV->height;

	if (color_matrix == 601) {  /* BT.601 matrix */
		for (k = 0; k < height; k++) {
			for (j = 0; j < width; j++) {
				r = imgYUV->Y[j + k * width] + 1.402 * (imgYUV->V[j + k * width] - 128);
				g = imgYUV->Y[j + k * width] - 0.344 * (imgYUV->U[j + k * width] - 128) - 0.714 * (imgYUV->V[j + k * width] - 128);
				b = imgYUV->Y[j + k * width] + 1.772 * (imgYUV->U[j + k * width] - 128);
				imgRGB->R[j + k * width] = rounding(r);
				imgRGB->G[j + k * width] = rounding(g);
				imgRGB->B[j + k * width] = rounding(b);
			}
		}
	}
	else {  /* BT.709 matrix */
		for (k = 0; k < height; k++) {
			for (j = 0; j < width; j++) {
				r = imgYUV->Y[j + k * width] + 1.5748 * (imgYUV->V[j + k * width] - 128);
				g = imgYUV->Y[j + k * width] - 0.1873 * (imgYUV->U[j + k * width] - 128) - 0.4681 * (imgYUV->V[j + k * width] - 128);
				b = imgYUV->Y[j + k * width] + 1.8556 * (imgYUV->U[j + k * width] - 128);
				imgRGB->R[j + k * width] = rounding(r);
				imgRGB->G[j + k * width] = rounding(g);
				imgRGB->B[j + k * width] = rounding(b);
			}
		}
	}

}

//----------------------------------------------------------------
//	Function-ID	RGB2YUVm_image
//	Function-Name	
//	Abstract	IMG_RGB型構造体の画像データに対し，IMG_RGB型構造体のYUV444への変換を行う
//
//	Argument	IMG_RGB *imgRGB: 原画像データ(444, input)
//		        IMG_COMMON *imgYUV: 処理画像データ(444, output)
//		        int color_matrix: 色変換マトリクスパラメータ（601 or 709) (input)
//			                       601: BT.601準拠(Y/Cb/Cr)
//                                 709: BT.709準拠(Y/Pb/Pr)
//	Return-Value	
//
//----------------------------------------------------------------

void  RGB2YUVm_image(IMG_RGB* imgRGB, IMG_COMMON* imgYUV, int color_matrix)
{

	int j, k;
	int width, height;
	double y, u, v;

	width = imgRGB->width;
	height = imgRGB->height;

	if (color_matrix == 601) {  /* BT.601 matrix */
		for (k = 0; k < height; k++) {
			for (j = 0; j < width; j++) {
				y = 0.299 * imgRGB->R[j + k * width] + 0.587 * imgRGB->G[j + k * width] + 0.114 * imgRGB->B[j + k * width];
				u = -0.169 * imgRGB->R[j + k * width] - 0.331 * imgRGB->G[j + k * width] + 0.5 * imgRGB->B[j + k * width];
				v = 0.5 * imgRGB->R[j + k * width] - 0.419 * imgRGB->G[j + k * width] - 0.081 * imgRGB->B[j + k * width];
				imgYUV->Y[j + k * width] = rounding(y);       /* Y conponent */
				imgYUV->U[j + k * width] = rounding(u + 128);   /* U conponent */
				imgYUV->V[j + k * width] = rounding(v + 128);   /* V conponent */
			}
		}
	}
	else {  /* BT.709 matrix */
		for (k = 0; k < height; k++) {
			for (j = 0; j < width; j++) {
				y = 0.2126 * imgRGB->R[j + k * width] + 0.7152 * imgRGB->G[j + k * width] + 0.0722 * imgRGB->B[j + k * width];
				u = -0.1146 * imgRGB->R[j + k * width] - 0.3854 * imgRGB->G[j + k * width] + 0.5 * imgRGB->B[j + k * width];
				v = 0.5 * imgRGB->R[j + k * width] - 0.4542 * imgRGB->G[j + k * width] - 0.0458 * imgRGB->B[j + k * width];
				imgYUV->Y[j + k * width] = rounding(y);       /* Y conponent */
				imgYUV->U[j + k * width] = rounding(u + 128);   /* U conponent */
				imgYUV->V[j + k * width] = rounding(v + 128);   /* V conponent */
			}
		}
	}

}

//----------------------------------------------------------------
//	Function-ID	RGB2HSVm_image
//	Function-Name	
//	Abstract	IMG_COMMON型構造体の画像データ(YUV444）に対し，IMG_RGB型構造体のRGBへの変換を行う
//
//	Argument	IMG_COMMON *imgYUV: 原画像データ(input)
//		        IMG_RGB *imgRGB: 処理画像データ(output)
//		        int color_matrix: 色変換マトリクスパラメータ（601 or 709) (input)
//			                       601: BT.601準拠(Y/Cb/Cr)
//                                 709: BT.709準拠(Y/Pb/Pr)
//	Return-Value	
//	
//----------------------------------------------------------------

void  RGB2HSVm_image(IMG_RGB* imgRGB, IMG_HSV* imgHSV, int width, int height)
{

	int j, k;
	double r, g, b;
	int h = 0;
	std::vector<double> vec(3);

	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			vec = { (double)imgRGB->R[j + k * width], (double)imgRGB->G[j + k * width], (double)imgRGB->B[j + k * width] };
			sort(vec.begin(), vec.end());
			r = imgRGB->R[j + k * width];
			g = imgRGB->G[j + k * width];
			b = imgRGB->B[j + k * width];
			if (vec[0] == vec[2]) {
				imgHSV->H[j + k * width] = 0;
				imgHSV->S[j + k * width] = 0;
				imgHSV->V[j + k * width] = (int)vec[0];
				continue;
			}
			else if (vec[2] == r) {
				h = (int)(60 * (g - b) / (vec[2] - vec[0]));

				if (h < 0) h += 360;
			}
			else if (vec[2] == g) {
				h = (int)(60 * (b - r) / (vec[2] - vec[0]) + 120);
			}
			else if (vec[2] == b) {
				h = (int)(60 * (r - g) / (vec[2] - vec[0]) + 240);
			}
			imgHSV->H[j + k * width] = h;
			imgHSV->S[j + k * width] = (int)((vec[2] - vec[0]) / vec[2] * 255);
			imgHSV->V[j + k * width] = (int)vec[2];
		}
	}
}


void  HSV2RGBm_image(IMG_HSV* imgHSV, IMG_RGB* imgRGB, int width, int height)
{

	int j, k;
	double min, max;

	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			max = imgHSV->V[j + k * width];
			min = max - (double)imgHSV->S[j + k * width] / 255 * max;

			if (imgHSV->H[j + k * width] >= 0 && imgHSV->H[j + k * width] < 60) {
				imgRGB->R[j + k * width] = (int)max;
				imgRGB->G[j + k * width] = (int)((double)imgHSV->H[j + k * width] / 60 * (max - min) + min);
				imgRGB->B[j + k * width] = (int)min;
			}
			else if (imgHSV->H[j + k * width] >= 60 && imgHSV->H[j + k * width] < 120) {
				imgRGB->R[j + k * width] = (int)((double)(120 - imgHSV->H[j + k * width]) / 60 * (max - min) + min);
				imgRGB->G[j + k * width] = (int)max;
				imgRGB->B[j + k * width] = (int)min;
			}
			else if (imgHSV->H[j + k * width] >= 120 && imgHSV->H[j + k * width] < 180) {
				imgRGB->R[j + k * width] = (int)min;
				imgRGB->G[j + k * width] = (int)max;
				imgRGB->B[j + k * width] = (int)((double)(imgHSV->H[j + k * width] - 120) / 60 * (max - min) + min);
			}
			else if (imgHSV->H[j + k * width] >= 180 && imgHSV->H[j + k * width] < 240) {
				imgRGB->R[j + k * width] = (int)min;
				imgRGB->G[j + k * width] = (int)((double)(240 - imgHSV->H[j + k * width]) / 60 * (max - min) + min);
				imgRGB->B[j + k * width] = (int)max;
			}
			else if (imgHSV->H[j + k * width] >= 240 && imgHSV->H[j + k * width] < 300) {
				imgRGB->R[j + k * width] = (int)((double)(imgHSV->H[j + k * width] - 240) / 60 * (max - min) + min);
				imgRGB->G[j + k * width] = (int)min;
				imgRGB->B[j + k * width] = (int)max;
			}
			else if (imgHSV->H[j + k * width] >= 300 && imgHSV->H[j + k * width] < 360) {
				imgRGB->R[j + k * width] = (int)max;
				imgRGB->G[j + k * width] = (int)min;
				imgRGB->B[j + k * width] = (int)((double)(360 - imgHSV->H[j + k * width]) / 60 * (max - min) + min);
			}

		}
	}

}



//----------------------------------------------------------------
//	Function-ID	YUV2RGBm_image
//	Function-Name	
//	Abstract	IMG_COMMON型構造体の画像データ(YUV444）に対し，IMG_RGB型構造体のRGBへの変換を行う
//
//	Argument	IMG_COMMON *imgYUV: 原画像データ(input)
//		        IMG_RGB *imgRGB: 処理画像データ(output)
//		        int color_matrix: 色変換マトリクスパラメータ（601 or 709) (input)
//			                       601: BT.601準拠(Y/Cb/Cr)
//                                 709: BT.709準拠(Y/Pb/Pr)
//	Return-Value	
//	
//----------------------------------------------------------------

void  YUV2HSVm_image(IMG_COMMON* imgYUV, IMG_HSV* imgHSV)
{

	int j, k;
	int width, height;
	int u, v;

	width = imgYUV->width;
	height = imgYUV->height;

	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			u = imgYUV->U[j + k * width] - 128;
			v = imgYUV->V[j + k * width] - 128;
			if (atan2(v, u) >= 0) imgHSV->H[j + k * width] = (int)(atan2(v, u) * 180 / PI);
			else imgHSV->H[j + k * width] = (int)(atan2(v, u) * 180 / PI + 360);
			imgHSV->S[j + k * width] = (int)(sqrt(pow(u, 2) + pow(v, 2)) * 255 / 100);
			imgHSV->V[j + k * width] = imgYUV->Y[j + k * width];

		}
	}


}

//----------------------------------------------------------------
//	Function-ID	RGB2YUVm_image
//	Function-Name	
//	Abstract	IMG_RGB型構造体の画像データに対し，IMG_RGB型構造体のYUV444への変換を行う
//
//	Argument	IMG_RGB *imgRGB: 原画像データ(444, input)
//		        IMG_COMMON *imgYUV: 処理画像データ(444, output)
//		        int color_matrix: 色変換マトリクスパラメータ（601 or 709) (input)
//			                       601: BT.601準拠(Y/Cb/Cr)
//                                 709: BT.709準拠(Y/Pb/Pr)
//	Return-Value	
//
//----------------------------------------------------------------

void  HSV2YUVm_image(IMG_HSV* imgHSV, IMG_COMMON* imgYUV)
{

	int j, k;
	int width, height;
	double h;
	int s;

	width = imgHSV->width;
	height = imgHSV->height;

	for (k = 0; k < height; k++) {
		for (j = 0; j < width; j++) {
			if (imgHSV->H[j + k * width] <= 180) h = (double)imgHSV->H[j + k * width] * PI / 180;
			else h = (double)((imgHSV->H[j + k * width] - 360) * PI / 180);
			s = imgHSV->S[j + k * width] * 100 / 255;
			imgYUV->Y[j + k * width] = imgHSV->V[j + k * width];       /* Y conponent */
			imgYUV->U[j + k * width] = (int)(s * cos(h) + 128);   /* U conponent */
			imgYUV->V[j + k * width] = (int)(s * sin(h) + 128);   /* V conponent */
		}
	}

}

//------------------------------------------------------------------------------
//	Function-ID	main
//	Function-Name	
//	Abstract	 YUV画像をBitmap画像に変換する
//               
//	Argument	nt	argc		= パラメータ数
//			char	*argv[]		= パラメータ文字列群
//	Return-Value	int			= 返却値
//	Special_Desc
//------------------------------------------------------------------------------

int main(int    argc, char* argv[])
{
	IMG_COMMON* img, * imgYUV;
	IMG_RGB* imgRGB;

	FILE* fp_in, * fp_outb;
	char outbfname[512], openfname[512];
	int width, height, size;
	int color_space, color_format;
	std::vector<std::string> files;

	/* パラメータチェック */
	if (argc < 6) {
		fprintf(stderr, "Usage: %s infile outfile width height color_space color_format \n", argv[0]);
		printf("        opendirectory: open directory path \n");
		printf("        width: horizontal picture size  \n");
		printf("        height: vertical picture size  \n");
		printf("        color_space: color space(601 or 709) \n");
		printf("        color_format: color format(420 or 444) \n");
		exit(-1);
	}

	/* パラメータ設定 */
	fs::path opendpath(argv[1]);      /* 入力ディレクトリ名 */
	width = atoi(argv[2]);          /* 水平方向画素数 */
	height = atoi(argv[3]);         /* 垂直方向画素数 */
	color_space = atoi(argv[4]);         /* YUV-RGB変換マトリクス（601 or 709） */
	color_format = atoi(argv[5]);         /* 入力YUVの色フォーマット（420 or 444） */

	size = width * height;

	printf(" -- open directory path= %s \n", opendpath.c_str());
	printf(" -- width= %d \n", width);
	printf(" -- height= %d \n", height);
	printf(" -- color_space= %d \n", color_space);
	printf(" -- color_format= %d \n", color_format);

	if (fs::is_directory(opendpath)) {
auto dir_it = fs::directory_iterator(opendpath);
for (auto& p : dir_it) {
	files.push_back(p.path().string());
}
	}

	for (std::string file : files) printf("%s\n", file.c_str());

	int n = 1;
	for (std::string file : files) {

		/* 画像構造体メモリ確保 */
		img = alloc_IMG_COMMON(width, height, color_format);
		imgYUV = alloc_IMG_COMMON(width, height, 444);
		imgRGB = alloc_IMG_RGB(width, height);

		/* 画像ファイルをオープンする */

		char* path_name = new char[file.size() + 1];
		std::char_traits<char>::copy(path_name, file.c_str(), file.size() + 1);
		strcpy(openfname, path_name);
		printf("%s\n", openfname);
		fp_in = open_image(openfname, "rb");

		std::string s;

		if (n < 10) s = "test" + std::to_string(n) + ".bmp";
		else s = "test" + std::to_string(n) + ".bmp";
		std::char_traits<char>::copy(outbfname, s.c_str(), s.size() + 1);
		fp_outb = open_image(outbfname, "wb");

		/* ----------------------------------------------------------------- */
		/* YUVファイルを読み込む */
		read_image(img, fp_in, odname);

		//---YUV変換

		if (color_format == 420) {
			trans_image_420to444(img, imgYUV, 1);    /* 420 →　444変換 */
		}
		else {
			copy_image(img, imgYUV, 1);
		}

		write_bmp(imgYUV, imgRGB, fp_outb);

		fclose(fp_in);
		fclose(fp_outb);
		free_IMG_COMMON(img);
		free_IMG_COMMON(imgYUV);
		free_IMG_RGB(imgRGB);


		delete[] path_name;
		n++;
	}

	return(0);
}

// Copyright (C) 2019
