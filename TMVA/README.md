# Examples on how to use the ROOT TMVA package

## Neural networks

### Classification

  * **NN_classifier_train.cpp** shows how to train a neural network, 
    starting from one signal and one background sample created in the program itself.
    The configuration parameters of the neutral network are the ROOT default ones,
    which are not optimised.
  * **NN_classifier_read.cpp** shows how to read an existing neural network, 
    produced by the NN_classifier_train.cpp program.
    The distribution of the discriminant is drawn for the signal and background events.
  * **NN_classifier_use.cpp** shows how to use the discriminant to select events as signal
    or background, respectively. Two histograms 
    (one for signal and one for background) are filled depending on the response of the NN
    trained with NN_classifier_train.cpp.