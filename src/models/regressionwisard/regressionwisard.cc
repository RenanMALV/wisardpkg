class RegressionWisard: public RegressionModel {
public:
  RegressionWisard(int addressSize) : addressSize(addressSize) {}

  RegressionWisard(int addressSize, bool completeAddressing, bool orderedMapping,
                  Mean* mean, int minZero=0, int minOne=0, int steps=0):
                  addressSize(addressSize), completeAddressing(completeAddressing),
                  orderedMapping(orderedMapping), mean(mean),
                  minZero(minZero), minOne(minOne), steps(steps), numberOfTrainings(0){

    srand(randint(0, 100000));

    checkMinZeroOne(minZero, minOne);
  }

  ~RegressionWisard(){
    rams.clear();
  }

  void train(const DataSet& dataset) {
    for (size_t i = 0; i < dataset.size(); i++){
      train(dataset[i], dataset.getY(i));
    }

    for (int j = 0; j < steps; j++){
      for (size_t i = 0; i < dataset.size(); i++){
        calculateFit(dataset[i], dataset.getY(i));
      }
      applyFit();
    }
  }

  void train(const BinInput& input, const double y){
    if (rams.empty()) setRAMShuffle(input.size());

    checkEntrySize(input.size());
    numberOfTrainings++;
    
    for (size_t i = 0; i < rams.size(); i++){
      rams[i].train(input, y);
    }
  }

  double predict(const BinInput& input) const {
    checkEntrySize(input.size());
    std::vector<regression_content_t> outputRams(rams.size());
    for (size_t i = 0; i < rams.size(); i++){
      outputRams[i] = rams[i].getVote(input);
    }
    return mean->calculate(outputRams);
  }

  std::vector<double> predict(const DataSet& dataset) const {
    std::vector<double> output(dataset.size());

    for (size_t i = 0; i < dataset.size(); i++){
      output[i] = predict(dataset[i]);
    }

    return output;
  }
  
  std::vector<double> getVotes(const BinInput& input) const {
    checkEntrySize(input.size());
    std::vector<double> output(rams.size());
    for (size_t i = 0; i < rams.size(); i++){
      output[i] = rams[i].getVote(input)[0];
    }
    return output;
  }

  void calculateFit(const BinInput& input, const double y){
    double yPredicted = predict(input);
    for (size_t i = 0; i < rams.size(); i++){
      rams[i].calculateFit(input, (y - yPredicted));
    }
  }

  void applyFit(){
    for (size_t i = 0; i < rams.size(); i++){
      rams[i].applyFit();
    }
  }

  int getNumberOfTrainings(){
    return numberOfTrainings;
  }

  long getsizeof() const{
    long size = sizeof(RegressionWisard);
    for(unsigned int i=0; i<rams.size(); i++){
      size += rams[i].getsizeof();
    }
    return size;
  }

  std::string json(std::string filename) const {
    return "";
  }

protected:
  void setRAMShuffle(int entrySize){
    this->entrySize = entrySize;
    checkAddressSize(entrySize, addressSize);
    int numberOfRAMS = entrySize / addressSize;
    int remain = entrySize % addressSize;
    int indexesSize = entrySize;
    if(completeAddressing && remain > 0) {
      numberOfRAMS++;
      indexesSize += addressSize-remain;
    }

    rams.resize(numberOfRAMS);
    std::vector<int> indexes(indexesSize);

    for (int i = 0; i < entrySize; i++){
      indexes[i]=i;
    }
    for (size_t i=entrySize; i<indexes.size(); i++){
      indexes[i] = randint(0, entrySize-1, false);
    }

    if(!orderedMapping)
      random_shuffle(indexes.begin(), indexes.end());

    for (size_t i=0; i<rams.size(); i++){
      std::vector<int> subIndexes(indexes.begin() + (i*addressSize), indexes.begin() + ((i+1)*addressSize));
      rams[i] = RegressionRAM(subIndexes, minZero, minOne);
    }
  }

  void checkEntrySize(const int entry) const {
    if(entrySize != entry){
      throw Exception("The entry size defined on creation of RAM is different of entry size given as input!");
    }
  }

  void checkAddressSize(const int entrySize, const int addressSize) const{
    if( addressSize < 2){
      throw Exception("The address size cann't be lesser than 2!");
    }
    if( entrySize < 2 ){
      throw Exception("The entry size cann't be lesser than 2!");
    }
    if( entrySize < addressSize){
      throw Exception("The address size cann't be bigger than entry size!");
    }
  }

  void checkMinZeroOne(int min0, int min1){
    if(min0+min1 > addressSize){
      throw Exception("minZero + minOne is bigger than addressSize!");
    }
  }
  
  int addressSize;
  bool completeAddressing;
  bool orderedMapping;
  Mean* mean;
  int minZero;
  int minOne;
  int steps;

  int numberOfTrainings;
  int entrySize;
  std::vector<RegressionRAM> rams;
};
