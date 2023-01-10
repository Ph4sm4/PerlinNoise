#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QWidget>
#include <QDebug>
#include <QHBoxLayout>
#include <QVector>
#include <math.h>
#include <QToolTip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->arraySize->setMinimum(128);
    ui->arraySize->setMaximum(256);

    ui->octaveNumber->setMinimum(1);
    ui->octaveNumber->setMaximum(ceil(log2(ui->arraySize->value())));

    ui->scalingBias->setMinimum(0.2);
    ui->scalingBias->setMaximum(10);
    ui->scalingBias->setSingleStep(0.1);
    ui->scalingBias->setValue(2);

    ui->oneD->setChecked(true);

    connect(ui->octaveNumber, SIGNAL(sliderMoved(int)), this, SLOT(showTooltip(int)));
    connect(ui->arraySize, SIGNAL(sliderMoved(int)), this, SLOT(showTooltip(int)));

    ui->width->setMinimum(256);
    ui->width->setMaximum(512);
    ui->width->setSingleStep(128);
    ui->width->setEnabled(false);
    ui->width->setStyleSheet("color: gray");

    this->setStyleSheet("background-color: #3e4444; color: white;");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showTooltip(const int& value)
{
    QToolTip::showText(QCursor::pos(), QString("%1").arg(value), nullptr);
}


void MainWindow::on_generateNoise_clicked()
{
    sampleSize1D = ui->arraySize->value();
    octaveNumber = ui->octaveNumber->value();
    scalingBias = ui->scalingBias->value();

    qDebug() << sampleSize1D << " " << octaveNumber;

    if(ui->oneD->isChecked()) {
        // allocating new memory for 1D arrays
        noiseSeed1D = new float [sampleSize1D];
        perlinNoise1D = new float [sampleSize1D];

        // populating noiseSeed with random numbers, range 0 to 1
        for(int i = 0; i < sampleSize1D; i++)
            noiseSeed1D[i] = (float)rand() / (float)RAND_MAX;

        generatePerlinNoise1D(sampleSize1D, noiseSeed1D, octaveNumber, perlinNoise1D, scalingBias);
        drawToScreen1D(perlinNoise1D, sampleSize1D);
    }
    else if(ui->twoD->isChecked()) {
        // allocating new memory for 2D arrays
        noiseSeed2D = new float [sampleWidth2D * sampleHeight2D];
        perlinNoise2D = new float [sampleWidth2D * sampleHeight2D];

        for(int i = 0; i < sampleWidth2D * sampleHeight2D; i++)
            noiseSeed2D[i] = (float)rand() / (float)RAND_MAX;

        generatePerlinNoise2D(sampleWidth2D, sampleHeight2D, noiseSeed2D, octaveNumber, perlinNoise2D, scalingBias);
        drawToScreen2D(perlinNoise2D, sampleWidth2D, sampleHeight2D);
    }
}

void MainWindow::generatePerlinNoise1D(int nCount, float *baseSeed, int nOctaves, float *noiseOutput, float bias)
{
    /*
     * nCount - size of the array (the number of points we wanna create)
     * baseSeed - base seed array of random values between 0 and 1
     * nOctaves - the number of iterations of the algorithm that we want to apply, maximum is log2(base seed array size)
     * noiseOutput - a pointer to the output array
     */
    for(int i = 0; i < nCount; i++) {
        float noiseValue = 0.f;

        float scalingFactorAcc = 0.f;
        float scalingFactor = 1.0f;

        for(int j = 0; j < nOctaves; j++) {
            int pitch = nCount >> j; // shifting bits to the right -> dividing by half/two each octave
            /*
             * the pitch is basically responsible for deciding how often do we want to choose a point, that is gonna enable us to calculate the value,
             * this way we can get more and more satisfing results
             */
            int firstPoint = (i / pitch) * pitch; // the first point is always going to be <= i
            int secondPoint = (pitch + firstPoint) % nCount; // modulo - because we want to loop around if we overflow the array
            /*
             * it is obligatory to choose 2 points, as in that one iteration we are calculating values for one single point, or element in the array,
             * as we know - we are "drawing" an invisible straight line between each value, this is why we can use linear interpolation between the
             * values from the seed array to get the proper noise value of that one particular cell
             *
             * as the octaves will grow, the lines will become shorter and shorter, to the point in which there is going to be a line between each calculated value
             * from the seed array
             */

            float HowFarIntoLine = (float)(i - firstPoint) / (float)pitch;
            /*
             * we need to know, how far are we into the invisible line between our current point (which we are calculating)
             * and the point that we have information about (the point that was chosen basing on the pitch, so how often
             * do we pick a point, and the information is simply its value)
             *
             * (float)(i - firstPoint) this will give us the value between 0 and the pitch
             *
             * for example, if current point's index is 87, and the octave is 3, then for our case (nCount/size of the array = 256)
             * the pitch is gonna be 256 / 2 / 2 which is 64, so upper expression will give us
             * (87 - 64 = 23) / 64 -> 0.359375
             * and that is how far are we into the "line" so using linear interpolation, we can now calculate the proper value of that point
             */
            float Interpolated = (1.0f - HowFarIntoLine) * baseSeed[firstPoint] + HowFarIntoLine * baseSeed[secondPoint];

            // now we can accumulate calculated values
            noiseValue += Interpolated * scalingFactor;

            scalingFactorAcc += scalingFactor;
            scalingFactor /= bias;
        }

        noiseOutput[i] = noiseValue / scalingFactorAcc;
        /*
         * by dividing by the scalingFactorAcc[umulation] we are making sure that the value range of our points is gonna
         * be in range from 0 to 1
         */
    }


}

void MainWindow::drawToScreen1D(float *noiseValues, int nCount)
{

    for(int i = 0; i < references.count(); i++) {
        this->layout()->removeWidget(static_cast<QWidget*>(references.at(i)));
        references.at(i)->deleteLater();
    }
    references.clear();

    for(int i = 0; i < nCount; i++) {
        QLabel* noiseCell = new QLabel();
        noiseCell->setGeometry(20 + i * 4, 570 - (int)(noiseValues[i] * 500), 3, noiseValues[i] * 500);
        noiseCell->setAlignment(Qt::AlignTop);

        noiseCell->setStyleSheet("background-color: #808080;");

        this->layout()->addWidget(static_cast<QWidget*>(noiseCell));

        references.append(noiseCell);
    }
}

void MainWindow::generatePerlinNoise2D(int nWidth, int nHeight, float *baseSeed, int nOctaves, float *noiseOutput, float bias)
{
    for(int x = 0; x < nWidth; x++){
        for(int y = 0; y < nHeight; y++) {
            float noiseValue = 0.f;

            float scalingFactorAcc = 0.f;
            float scalingFactor = 1.0f;

            for(int j = 0; j < nOctaves; j++) {
                int pitch = nWidth >> j;

                int pointX1 = (x / pitch) * pitch;
                int pointY1 = (y / pitch) * pitch;

                int pointX2 = (pointX1 + pitch) % nWidth;
                int pointY2 = (pointY1 + pitch) % nWidth;

                float HowFarIntoLineX = (float)(x - pointX1) / (float)pitch;
                float HowFarIntoLineY = (float)(y - pointY1) / (float)pitch;

                float InterpolatedX = (1.0f - HowFarIntoLineX) * baseSeed[pointY1 * nWidth + pointX1] + HowFarIntoLineX * baseSeed[pointY1 * nWidth + pointX2];
                float InterpolatedY = (1.0f - HowFarIntoLineX) * baseSeed[pointY2 * nWidth + pointX1] + HowFarIntoLineX * baseSeed[pointY2 * nWidth + pointX2];

                // now we can accumulate calculated values
                noiseValue += (HowFarIntoLineY * (InterpolatedY - InterpolatedX) + InterpolatedX) * scalingFactor;

                scalingFactorAcc += scalingFactor;
                scalingFactor /= bias;
            }

            noiseOutput[y * nWidth + x] = noiseValue / scalingFactorAcc;
        }
    }
}

void MainWindow::drawToScreen2D(float *noiseValues, int nWidth, int nHeight)
{
    for(int i = 0; i < references.count(); i++) {
        this->layout()->removeWidget(static_cast<QWidget*>(references.at(i)));
        references.at(i)->deleteLater();
    }
    references.clear();

    QImage image(nWidth, nHeight, QImage::Format_RGB888);
    QLabel* imageDisplayer = new QLabel();

    for(int x = 0; x < nWidth; x++) {
        for(int y = 0; y < nHeight; y++) {
            int noiseColor = (int)(noiseValues[y * nWidth + x] * 12.0f); // we can have more colors, with multiplying by 12 we have 12 possible colors
            QRgb properColor;

            switch(noiseColor) {
                case 0: properColor = qRgb(208,208,208); break;
                case 1: properColor = qRgb(190,190,190); break;
                case 2: properColor = qRgb(176,176,176); break;
                case 3: properColor = qRgb(168,168,168); break;
                case 4: properColor = qRgb(160,160,160); break;
                case 5: properColor = qRgb(136,136,136); break;
                case 6: properColor = qRgb(120,120,120); break;
                case 7: properColor = qRgb(105,105,105); break;
                case 8: properColor = qRgb(96,96,96); break;
                case 9: properColor = qRgb(80,80,80); break;
                case 10: properColor = qRgb(64,64,64); break;
                case 11: properColor = qRgb(48,48,48); break;
                default: properColor = qRgb(24,24,24); break; // 12
            }
            image.setPixel(x, y, properColor);
        }
    }
    imageDisplayer->setPixmap(QPixmap::fromImage(image));
    this->layout()->addWidget(static_cast<QWidget*>(imageDisplayer));
    imageDisplayer->move(50, 50);

    references.append(imageDisplayer);
}




void MainWindow::on_octaveNumber_valueChanged(int value)
{
    octaveNumber = value;
    if(noiseSeed1D && perlinNoise1D && ui->oneD->isChecked()) {
        generatePerlinNoise1D(sampleSize1D, noiseSeed1D, octaveNumber, perlinNoise1D, scalingBias);
        drawToScreen1D(perlinNoise1D, sampleSize1D);
    }
    else if (noiseSeed2D && perlinNoise2D && ui->twoD->isChecked()) {
        generatePerlinNoise2D(sampleWidth2D, sampleHeight2D, noiseSeed2D, octaveNumber, perlinNoise2D, scalingBias);
        drawToScreen2D(perlinNoise2D, sampleWidth2D, sampleHeight2D);
    }
}


void MainWindow::on_arraySize_valueChanged(int value)
{
    ui->octaveNumber->setMaximum(ceil(log2(value)));
}


void MainWindow::on_scalingBias_valueChanged(double arg1)
{
    scalingBias = arg1;
    if(noiseSeed1D && perlinNoise1D && ui->oneD->isChecked()) {
        generatePerlinNoise1D(sampleSize1D, noiseSeed1D, octaveNumber, perlinNoise1D, scalingBias);
        drawToScreen1D(perlinNoise1D, sampleSize1D);
    }
    else if (noiseSeed2D && perlinNoise2D && ui->twoD->isChecked()) {
        generatePerlinNoise2D(sampleWidth2D, sampleHeight2D, noiseSeed2D, octaveNumber, perlinNoise2D, scalingBias);
        drawToScreen2D(perlinNoise2D, sampleWidth2D, sampleHeight2D);
    }
}


void MainWindow::on_twoD_toggled(bool checked)
{
    if(checked)
    {
        ui->width->setEnabled(true);
        ui->width->setStyleSheet("color: white");
    }
     else
    {
        ui->width->setEnabled(false);
        ui->width->setStyleSheet("color: gray");
    }
}


void MainWindow::on_width_valueChanged(int arg1)
{
    sampleWidth2D = arg1;
    sampleHeight2D = arg1;
    ui->sizeDisplayer->setText("Image size: " + QString::number(arg1) + " x " + QString::number(arg1));
}

