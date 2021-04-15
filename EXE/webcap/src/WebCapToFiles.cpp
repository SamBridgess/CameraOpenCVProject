#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <filesystem>
#include "opencv2/core/utility.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <windows.h>

using namespace cv;
using namespace std;

const string currentDateTime() 
{
    // строка-метка времени для имени файла
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d@%H-%M-%S", &tstruct);
    return buf;
}


auto start = chrono::system_clock::now(); // инициализация значения времени
bool GetTimeEvent(int seconds)
{
    // поиск события - превышение времни
    auto end = chrono::system_clock::now();

    chrono::duration<double> elapsed_seconds = end - start;
    if (elapsed_seconds.count() > seconds) {
        start = chrono::system_clock::now(); // обновление стартового значения времени
        return 1;
    }
    return 0;
}

void OpenVideoFile(VideoWriter& video, int codec, int frame_width, int frame_height, string video_directory, int fps)
{
    // открытие видеофоайла на запись
    string const basename = currentDateTime() + ".avi";
    cout << "write file: " << basename << endl;
    string const filename = video_directory + basename;
    video.open(filename, codec, fps, Size(frame_width, frame_height), true);
}


int main(int ac, char *av[]) {

    VideoCapture cap;
    int camNum = 0; // номер видеопотока
    bool paused = false;
    cap.open(camNum);
    Mat frame;
    
    // задание выходной директории (текущая + \video\)
    string const video_directory = filesystem::current_path().string() + "\\video\\";
	cout << "write files to directory: " << video_directory << endl;
	::CreateDirectoryA(video_directory.c_str(), NULL);

    // gпараметры видео от камеры
    int codec = VideoWriter::fourcc('X', 'V', 'I', 'D'); //cap.get(CAP_PROP_FOURCC);
    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);

    // получение данных из командной строки
    int fps = 20; 
    int durationOneFileSeconds = 60; // количество секунд видео в одном файле
	if (ac > 1) fps = stoi(av[1]);
	if (ac > 2) durationOneFileSeconds = stoi(av[2]);
	cout << "rate: " << fps << " fps" << endl;
	cout << "limit: " << durationOneFileSeconds << " sec" << endl;

    //проверка корректности подключения к камере
    if (!cap.isOpened())
    {
        cout << "could not initialize capturing" << endl;
        return 0;
    }
    namedWindow("Cam", 0);

    // открытие файла на запись
    VideoWriter video;
    OpenVideoFile(video, codec, frame_width, frame_height, video_directory, fps);

    while (true) {
        if (!paused) {
            cap >> frame;
            if (frame.empty())
                break;

            // переоткрытие нового файлв\а на запись
            if (GetTimeEvent(durationOneFileSeconds)) {
                video.release();
                OpenVideoFile(video, codec, frame_width, frame_height, video_directory, fps);
            }

            // запись очередного кадра в файл
            video.write(frame);
        }

        // отображение очередного кадра
        imshow("Cam", frame);

        // Обработка нажатия клавиш
        char c = (char)waitKey(10);
        if (c == 27)
            break;
        switch (c)
        {
        case 'p':
            paused = !paused;
            break;
        default:
            ;
        }
    }
    return 0;
}