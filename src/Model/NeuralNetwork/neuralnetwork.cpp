﻿#include "neuralnetwork.h"


// STL
#include <fstream>

// Custom
#include "../src/Model/Layer/layer.h"
#include "../src/Model/Neuron/neuron.h"
#include "../src/Model/Connection/connection.h"
#include "../src/View/MainWindow/mainwindow.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------


NeuralNetwork::NeuralNetwork(const std::vector<unsigned int>& vArchitecture, MainWindow* pMainWindow)
{
    this ->pMainWindow = pMainWindow;

    pRndGen            = new std::mt19937_64( std::random_device{}() );

    fTrainingSpeed     = 0.5f;

    bTraining          = false;


    loadTrainingSamples();
    loadTestingSamples();

    setArchitecture(vArchitecture);
}

NeuralNetwork::NeuralNetwork(MainWindow* pMainWindow)
{
    this ->pMainWindow = pMainWindow;

    pRndGen            = new std::mt19937_64( std::random_device{}() );

    fTrainingSpeed     = 0.5f;

    loadTrainingSamples();
    loadTestingSamples();
}

void NeuralNetwork::setArchitecture(const std::vector<unsigned int> &vArchitecture)
{
    // Setup layers.

    for (size_t i = 0;   i < vArchitecture .size();   i++)
    {
        vLayers .push_back( new Layer(vArchitecture[i]) );
    }




    // Setup connections.

    for (size_t i = 0;   i < vLayers .size();   i++)
    {
        if ( i != 0 )
        {
            vLayers[i] ->connectToPrevLayer( vLayers[i - 1] );
        }

        if ( i != (vLayers .size() - 1) )
        {
           vLayers[i] ->connectToNextLayer( vLayers[i + 1] );
        }
    }
}

void NeuralNetwork::setRandomWeights()
{
    std::uniform_real_distribution<float> urd(-0.5f, 0.5f);

    for (size_t i = 0;   i < vLayers .size();   i++)
    {
        const std::vector<Neuron*>& vNeurons = vLayers[i] ->getNeurons();

        for (size_t k = 0;   k < vNeurons .size();   k++)
        {
            const std::vector<Connection*>& vInConnections  = vNeurons[k] ->getInConnections();
            const std::vector<Connection*>& vOutConnections = vNeurons[k] ->getOutConnections();


            for (size_t n = 0;   n < vInConnections .size();   n++)
            {
                vInConnections[n] ->setupStartWeight( urd(*pRndGen) );
            }


            for (size_t n = 0;   n < vOutConnections .size();   n++)
            {
                vOutConnections[n] ->setupStartWeight( urd(*pRndGen) );
            }
        }
    }
}

void NeuralNetwork::showTrainingSample(size_t i)
{
    pMainWindow ->drawSample(true, i, vTrainingSamples[i] .iSampleValue, vTrainingSamples[i] .pixels);
}

void NeuralNetwork::showTestingSample(size_t i)
{
    pMainWindow ->drawSample(false, i, vTestingSamples[i] .iSampleValue, vTestingSamples[i] .pixels);
}

void NeuralNetwork::startTraining()
{
    bTraining = true;

    double dLowestCost = 1.0;

    size_t iRightAnswersInARow = 0;

    size_t iGlobalI = 0;

    while (bTraining)
    {
        for (size_t i = 0;   (i < vTrainingSamples .size()) && (bTraining);   i++)
        {
            // Convert input to a one-dimensional [0.0 ... 1.0] vector.

            std::vector<float> vTrainingSample;

            for (size_t row = 0;   row < vTrainingSamples[i] .pixels .size();   row++)
            {
                for (size_t column = 0;   column < vTrainingSamples[i] .pixels[row] .size();   column++)
                {
                    float fInput = vTrainingSamples[i] .pixels[row][column] / static_cast <float> (UCHAR_MAX);

                    vTrainingSample .push_back( fInput );
                }
            }



            // Give the input to the neurons.

            vLayers[0] ->setInputForNeurons (vTrainingSample);

            for (size_t j = 1;   j < vLayers .size();   j++)
            {
                vLayers[j] ->calculateResult();
            }




            // Get the error.

            std::vector<Neuron*> vOutputNeurons = vLayers .back() ->getNeurons();

            double dCost = 0.0;

            size_t iBiggestOutput = 0;

            for (size_t k = 0;   k < vOutputNeurons .size();   k++)
            {
                if (k != 0)
                {
                    if ( vOutputNeurons[k] ->getOutputSignal() > vOutputNeurons[iBiggestOutput] ->getOutputSignal() )
                    {
                        iBiggestOutput = k;
                    }
                }



                float fDesired = 0.0f;

                if ( k == vTrainingSamples[i] .iSampleValue )
                {
                    fDesired = 1.0f;
                }

                double dResult = static_cast <double> (fDesired) - static_cast<double> ( vOutputNeurons[k] ->getOutputSignal() );

                dCost += pow( (dResult), 2 );
                //dCost += abs(dResult);

                vOutputNeurons[k] ->setError( fTrainingSpeed * static_cast<float> (dResult) );
            }

            if ( iBiggestOutput == vTrainingSamples[i] .iSampleValue )
            {
                iRightAnswersInARow++;
            }
            else
            {
                iRightAnswersInARow = 0;
            }



            // Backpropogation.

            for (size_t k = vLayers .size() - 2;   k > 0;   k--)
            {
                vLayers[k] ->calculateError();
            }

            // Recalculate the weights.

            for (size_t k = 1;   k < vLayers .size();   k++)
            {
                vLayers[k] ->recalculateWeights(fTrainingSpeed);
            }



            dCost /= 2;
            //dCost /= vOutputNeurons .size();

            if (dCost < dLowestCost)
            {
                dLowestCost = dCost;
            }
            else
            {
                dCost = dLowestCost;
            }

            iGlobalI++;

            //pMainWindow ->addTrainingCostValue( static_cast<double> (iGlobalI + 1), static_cast <double> (dCost) );
            pMainWindow ->addTrainingCostValue( static_cast<double> (iGlobalI + 1), static_cast <double> (iRightAnswersInARow) );

            if ( i % 100 == 0 )
            {
                pMainWindow ->processEvents();
            }
        }
    }
}

void NeuralNetwork::stopTraining()
{
    bTraining = false;
}

void NeuralNetwork::startTesting()
{
    size_t iCountRightAnswers = 0;

    for (size_t i = 0;   i < vTestingSamples .size();   i++)
    {
        // Convert input to a one-dimensional [0.0 ... 1.0] vector.

        std::vector<float> vTestingSample;

        for (size_t row = 0;   row < vTestingSamples[i] .pixels .size();   row++)
        {
            for (size_t column = 0;   column < vTestingSamples[i] .pixels[row] .size();   column++)
            {
                float fInput = vTestingSamples[i] .pixels[row][column] / static_cast <float> (UCHAR_MAX);

                vTestingSample .push_back( fInput );
            }
        }



        // Give the input to the neurons.

        vLayers[0] ->setInputForNeurons (vTestingSample);

        for (size_t j = 1;   j < vLayers .size();   j++)
        {
            vLayers[j] ->calculateResult();
        }




        // Get the biggest output.

        std::vector<Neuron*> vOutputNeurons = vLayers .back() ->getNeurons();

        size_t iBiggestOutputIndex = 0;

        for (size_t k = 1;   k < vOutputNeurons .size();   k++)
        {
            if ( vOutputNeurons[k] ->getOutputSignal() > vOutputNeurons[iBiggestOutputIndex] ->getOutputSignal() )
            {
                iBiggestOutputIndex = k;
            }
        }




        // Result.

        if ( iBiggestOutputIndex == vTestingSamples[i] .iSampleValue )
        {
            iCountRightAnswers++;
        }

        double dPercent = (iCountRightAnswers / static_cast <double>(i + 1)) * 100.0;

        pMainWindow ->addTestingResult( static_cast<double> (i + 1), dPercent );


        if ( i % 100 == 0 )
        {
            pMainWindow ->processEvents();
        }
    }
}

void NeuralNetwork::answer(std::vector< std::vector<unsigned char> > vInput)
{
    // Convert input to a one-dimensional [0.0 ... 1.0] vector.

    std::vector<float> vTestingSample;

    for (size_t row = 0;   row < vInput .size();   row++)
    {
        for (size_t column = 0;   column < vInput[row] .size();   column++)
        {
            float fInput = vInput[row][column] / static_cast <float> (UCHAR_MAX);

            vTestingSample .push_back( fInput );
        }
    }



    // Give the input to the neurons.

    vLayers[0] ->setInputForNeurons (vTestingSample);

    for (size_t j = 1;   j < vLayers .size();   j++)
    {
        vLayers[j] ->calculateResult();
    }




    // Get the biggest output.

    std::vector<Neuron*> vOutputNeurons = vLayers .back() ->getNeurons();

    size_t iBiggestOutputIndex = 0;

    for (size_t k = 1;   k < vOutputNeurons .size();   k++)
    {
        if ( vOutputNeurons[k] ->getOutputSignal() > vOutputNeurons[iBiggestOutputIndex] ->getOutputSignal() )
        {
            iBiggestOutputIndex = k;
        }
    }




    // Result.

    pMainWindow ->answer(iBiggestOutputIndex);
}

void NeuralNetwork::saveTraining(std::wstring sPath)
{
    std::ofstream file (sPath, std::ios::binary);




    // Save architecture.

    unsigned char cArcSize = static_cast <unsigned char> ( vLayers .size() );
    file .write( reinterpret_cast<char*>(&cArcSize), sizeof(cArcSize) );

    for (size_t i = 0;   i < vLayers .size();   i++)
    {
        unsigned int iNeuronCount = static_cast <unsigned int>( vLayers[i] ->getNeurons() .size() );

        file .write( reinterpret_cast<char*>(&iNeuronCount), sizeof(iNeuronCount) );
    }




    // Save bias.

    float fBias = vLayers[0] ->getNeurons()[0] ->getBias();

    file .write( reinterpret_cast<char*>(&fBias), sizeof(fBias) );




    // Save training speed.

    file .write( reinterpret_cast<char*>(&fTrainingSpeed), sizeof(fTrainingSpeed) );




    // Save weights.

    for (size_t i = 0;   i < vLayers .size();   i++)
    {
        for (size_t j = 0;   j < vLayers[i] ->getNeurons() .size();   j++)
        {
            std::vector<Connection*> vInConnections  = vLayers[i] ->getNeurons()[j] ->getInConnections();
            std::vector<Connection*> vOutConnections = vLayers[i] ->getNeurons()[j] ->getOutConnections();

            for (size_t k = 0;   k < vInConnections .size();   k++)
            {
                file .write( reinterpret_cast<char*>(&vInConnections[k] ->fWeight), sizeof(vInConnections[k] ->fWeight) );
            }

            for (size_t k = 0;   k < vOutConnections .size();   k++)
            {
                file .write( reinterpret_cast<char*>(&vOutConnections[k] ->fWeight), sizeof(vOutConnections[k] ->fWeight) );
            }
        }
    }



    file .close();
}

void NeuralNetwork::openTraining(std::wstring sPath)
{
    std::ifstream file (sPath, std::ios::binary);


    // Save architecture.

    unsigned char cArcSize = 0;
    file .read( reinterpret_cast<char*>(&cArcSize), sizeof(cArcSize) );

    std::vector<unsigned int> vArchitecture;

    for (size_t i = 0;   i < cArcSize;   i++)
    {
        unsigned int iNeuronCount = 0;

        file .read( reinterpret_cast<char*>(&iNeuronCount), sizeof(iNeuronCount) );

        vArchitecture .push_back(iNeuronCount);
    }

    setArchitecture(vArchitecture);




    // Save bias.

    float fBias = 0;

    file .read( reinterpret_cast<char*>(&fBias), sizeof(fBias) );

    setBiasForAll(fBias);




    // Save training speed.

    float fNewTrainingSpeed = 0;

    file .read( reinterpret_cast<char*>(&fNewTrainingSpeed), sizeof(fNewTrainingSpeed) );

    setTrainingSpeed(fNewTrainingSpeed);




    // Save weights.

    for (size_t i = 0;   i < vLayers .size();   i++)
    {
        for (size_t j = 0;   j < vLayers[i] ->getNeurons() .size();   j++)
        {
            std::vector<Connection*> vInConnections  = vLayers[i] ->getNeurons()[j] ->getInConnections();
            std::vector<Connection*> vOutConnections = vLayers[i] ->getNeurons()[j] ->getOutConnections();

            for (size_t k = 0;   k < vInConnections .size();   k++)
            {
                file .read( reinterpret_cast<char*>(&vInConnections[k] ->fWeight), sizeof(vInConnections[k] ->fWeight) );
            }

            for (size_t k = 0;   k < vOutConnections .size();   k++)
            {
                file .read( reinterpret_cast<char*>(&vOutConnections[k] ->fWeight), sizeof(vOutConnections[k] ->fWeight) );
            }
        }
    }


    file .close();
}

void NeuralNetwork::setBiasForAll(float fBias)
{
    for (size_t i = 0;   i < vLayers .size();   i++)
    {
        vLayers[i] ->setBiasToAll(fBias);
    }
}

void NeuralNetwork::setTrainingSpeed(float fSpeed)
{
    if      (fSpeed > 1.0f)
    {
        fTrainingSpeed = 1.0f;
    }
    else if (fSpeed < 0.0f)
    {
        fTrainingSpeed = 0.0f;
    }
    else
    {
        fTrainingSpeed = fSpeed;
    }
}

void NeuralNetwork::loadTrainingSamples()
{
    std::ifstream file("train-images", std::ios::binary);


    // magic number

    int iNumber = 0;
    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );


    // number of images

    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );


    // number of rows

    int iRows = 0;
    file .read( reinterpret_cast<char*>(&iRows), sizeof(iRows) );


    // number of columns

    int iColumns = 0;
    file .read( reinterpret_cast<char*>(&iColumns), sizeof(iColumns) );



    for (int n = 0;   n < iNumber;   n++)
    {
        std::vector< std::vector<unsigned char> > pixels;

        pixels .resize( iColumns );

        for (int i = 0;   i < iRows;   i++)
        {
            for (int j = 0;   j < iColumns;   j++)
            {
                unsigned char byte = 0;
                file .read( reinterpret_cast<char*>(&byte), sizeof(byte) );

                pixels[j] .push_back(byte);
            }
        }

        vTrainingSamples .push_back(ImageSample(pixels));
    }


    file .close();







    file.open("train-labels", std::ios::binary);


    // magic number

    iNumber = 0;
    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );


    // number of items

    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );



    for (int n = 0;   n < iNumber;   n++)
    {
        unsigned char byte = 0;
        file .read(reinterpret_cast<char*>(&byte), sizeof(byte));

        vTrainingSamples[n].iSampleValue = byte;
    }


    file .close();

}

void NeuralNetwork::loadTestingSamples()
{
    std::ifstream file("t10k-images", std::ios::binary);


    // magic number

    int iNumber = 0;
    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );


    // number of images

    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );


    // number of rows

    int iRows = 0;
    file .read( reinterpret_cast<char*>(&iRows), sizeof(iRows) );


    // number of columns

    int iColumns = 0;
    file .read( reinterpret_cast<char*>(&iColumns), sizeof(iColumns) );



    for (int n = 0;   n < iNumber;   n++)
    {
        std::vector< std::vector<unsigned char> > pixels;

        pixels .resize( iColumns );

        for (int i = 0;   i < iRows;   i++)
        {
            for (int j = 0;   j < iColumns;   j++)
            {
                unsigned char byte = 0;
                file .read( reinterpret_cast<char*>(&byte), sizeof(byte) );

                pixels[j] .push_back(byte);
            }
        }

        vTestingSamples .push_back(ImageSample(pixels));
    }


    file .close();







    file.open("t10k-labels", std::ios::binary);


    // magic number

    iNumber = 0;
    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );


    // number of items

    file .read( reinterpret_cast<char*>(&iNumber), sizeof(iNumber) );



    for (int n = 0;   n < iNumber;   n++)
    {
        unsigned char byte = 0;
        file .read(reinterpret_cast<char*>(&byte), sizeof(byte));

        vTestingSamples[n].iSampleValue = byte;
    }


    file .close();
}


NeuralNetwork::~NeuralNetwork()
{
    delete pRndGen;

    for (size_t i = 0;   i < vLayers .size();   i++)
    {
        delete vLayers[i];
    }
}
