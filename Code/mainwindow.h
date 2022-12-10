#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_generateNoise_clicked();

    void on_octaveNumber_valueChanged(int value);

    void on_arraySize_valueChanged(int value);

    void on_scalingBias_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;

    ////////////////////////////////////////////////////////////////////////////////////////
    /// \brief 1D perlin noise variables
    ///

    float *noiseSeed1D = nullptr; // base seed 1D array
    float *perlinNoise1D = nullptr; // populated 1D perlin noise values

    void generatePerlinNoise1D(int nCount, float *baseSeed, int nOctaves, float *noiseOutput, float bias);
    void drawToScreen1D(float *noiseValues, int nCount);

    int sampleSize1D; // size of the array
    int octaveNumber; // algorithm iteration number
    float scalingBias; // the divisor of the pitch (how often do we pick a point)

    QList<QLabel*> references; // label pointer reference list, in order to clear previously displayed ones

    ////////////////////////////////////////////////////////////////////////////////////////
    /// \brief 2D perlin noise variables
    ///


    float *noiseSeed2D = nullptr;
    float *perlinNoise2D = nullptr;

    int sampleHeight2D = 256; // height of the image that we will be generating
    int sampleWidth2D = 256; // width of the image that we will be generating

    void generatePerlinNoise2D(int nWidth, int nHeight, float *baseSeed, int nOctaves, float *noiseOutput, float bias);
    void drawToScreen2D(float *noiseValues, int nWidth, int nHeight);

private slots:
    void showTooltip(const int& value);
    void on_twoD_toggled(bool checked);
    void on_width_valueChanged(int arg1);
};
#endif // MAINWINDOW_H
