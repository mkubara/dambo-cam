
////////////////////////////////////////////////////////////
//----------------------------------------------------------
// ◆GPIOスイッチを押されたら、カメラで撮影するサンプル
//----------------------------------------------------------
#include <stdio.h>
#include <dirent.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <wiringPi.h>

#define PIN_LED 10
#define PIN_SW 9
#define PIN_BEEP 11


//----------------------------------------------------------
// 指定の周波数の音を鳴らす
//----------------------------------------------------------
int tone(int pin, int hz, int ms)
{
	int tm = 750000/hz;			// 便宜上早めに。
	int rep = ms*1000/(tm*2);
	
	int i;
	for(i=0;i<rep;i++){
		digitalWrite(pin,1);
		delayMicroseconds(tm);
		digitalWrite(pin,0);
		delayMicroseconds(tm);
	}
}

//----------------------------------------------------------
// GPIOセットアップ
//----------------------------------------------------------
int setupGpio()
{
	// GPIOライブラリの初期化
	if( wiringPiSetupGpio() == -1 ) return -1;
//	piHiPri(50);

	// LEDとビープ音は出力、スイッチは入力
	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_BEEP, OUTPUT);
	pinMode(PIN_SW, INPUT);

	// スイッチ入力はプルアップ
	pullUpDnControl (PIN_SW, PUD_UP);
	
	return 0;
}

//----------------------------------------------------------
// 指定のディレクトリ内のファイル数を返す
//----------------------------------------------------------
int countFiles(const char *dir)
{
	DIR *d;
	struct dirent *ent;
	
	// ディレクトリを開く
	if((d=opendir(dir))==NULL)	return -1;
	
	// すべてのファイルを検索し、その数をカウント
	int count = 0;
	while((ent=readdir(d))!=NULL){
		++count;
	}
	
	// 終了し、数を返す
	closedir(d);
	return count-2;
}

//----------------------------------------------------------
// カメラで撮影
//----------------------------------------------------------
int takePicture(const char *path, int w, int h)
{
	// カメラの初期化
	CvCapture *capture = cvCreateCameraCapture(-1);
	if (capture==NULL) {
		puts("*ERR* cvCreateCameraCapture");
		return -1;
	}
	// 解像度設定
	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, w);
	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, h);

	// カメラから画像を取得
	IplImage *frame = cvQueryFrame(capture);
	if( !frame ){
		puts("*ERR* cvQueryFrame");
		return -2;
	}

	// 画像データをJPEGファイルにして保存
	if( !cvSaveImage(path, frame, NULL) ){
		puts("*ERR* cvSaveImage");
		return -3;
	}
	
	// カメラを解放し終了
	cvReleaseCapture (&capture);
	return 0;
}

//----------------------------------------------------------
// main
//----------------------------------------------------------
int main(int argc, char **argv)
{
	// GPIO初期化
	if( setupGpio() < 0 ){
		return -1;
	}
	
	// スタンバイLEDをON
	digitalWrite(PIN_LED, 1);
	
	while(1){
		// シャッター待ち
		if( digitalRead(PIN_SW)!=0 ){
			continue;
		}

		// 撮影開始（LED消灯＋ピコ音）
		digitalWrite(PIN_LED, 0);
		tone(PIN_BEEP, 1319, 100);
		tone(PIN_BEEP, 1760, 100);
		
		// 保存ファイル名を決定し、撮影処理
		char path[256];
		sprintf(path, "photo/cam%05d.jpg", countFiles("photo"));
		takePicture(path, 640, 480);
		
		// 撮影終了（LED点灯＋ピピピポーン音）
		digitalWrite(PIN_LED, 1);
		tone(PIN_BEEP, 1975, 50);
		delay(50);
		tone(PIN_BEEP, 1975, 50);
		delay(50);
		tone(PIN_BEEP, 1975, 50);
		delay(50);
		tone(PIN_BEEP, 2093, 200);
	}
	
	return 0;
}
