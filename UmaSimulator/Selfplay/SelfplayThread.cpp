#include "../External/cnpy/cnpy.h"
#include "SelfplayThread.h"
using namespace std;

static bool randBool(mt19937_64& rand, double p)
{
  return rand() % 65536 < p * 65536;
}

static std::string generateHexFileName(std::mt19937_64& rand) {
  string s;
  s.assign(32, ' ');
  const char hexc[] = "0123456789abcdef";
  for (int i = 0; i < 32; i++) 
  {
    s[i] = hexc[rand() % 16];
  }

  return s;
}

SelfplayThread::SelfplayThread(SelfplayParam param, Model* model) :param(param), gameGenerator(param, model), search(model, param.batchsize, param.threadNumInner)
{
  std::random_device rd;
  rand = std::mt19937_64(rd());
  sampleData.resize(param.sampleNumEachFile);
}

void SelfplayThread::run()
{
  int fileNum = 1 + ((param.maxSampleNum - 1) / param.threadNum);
  for (int f = 0; f < fileNum; f++)
  {
    for (int g = 0; g < param.sampleNumEachFile; g++)
      sampleData[g] = generateSingleSample();
    writeDataToFile();
    cout << f << endl;
  }
}

TrainingSample SelfplayThread::generateSingleSample()
{
  std::exponential_distribution<double> expDistr(1.0);
  std::normal_distribution<double> normDistr(0.0, 1.0);
  std::uniform_real_distribution<double> uniDistr(0.0, 1.0);

  SearchParam sp;
  if (randBool(rand, param.maxDepth_fullProb))
    sp.maxDepth = TOTAL_TURN;
  else
  {
    sp.maxDepth = int(exp(param.maxDepth_logmean + normDistr(rand) * param.maxDepth_logstdev) + 0.5);
    if (sp.maxDepth < param.maxDepth_min)sp.maxDepth = param.maxDepth_min;
    if (sp.maxDepth > TOTAL_TURN)sp.maxDepth = TOTAL_TURN;
  }

  sp.maxRadicalFactor = param.radicalFactor_scale * (pow(uniDistr(rand) + 1e-10, -param.radicalFactor_pow) - 1);
  if (sp.maxRadicalFactor < 0)sp.maxRadicalFactor = 0;
  if (sp.maxRadicalFactor > param.radicalFactor_max)sp.maxRadicalFactor = param.radicalFactor_max;

  sp.samplingNum = int(exp(param.searchN_logmean + normDistr(rand) * param.searchN_logstdev) + 0.5);
  if (sp.samplingNum > param.searchN_max)sp.samplingNum = param.searchN_max;
  if (sp.samplingNum < param.searchN_min)sp.samplingNum = param.searchN_min;
  //让param.samplingNum是整batch
  {
    int batchEveryThread = (sp.samplingNum - 1) / (param.threadNumInner * param.batchsize) + 1;//相当于向上取整
    if (batchEveryThread <= 0)batchEveryThread = 1;
    int samplingNumEveryThread = param.batchsize * batchEveryThread;
    sp.samplingNum = param.threadNumInner * samplingNumEveryThread;
  }


  Game game = gameGenerator.get();
  search.setParam(sp);
  search.runSearch(game, rand);
  return search.exportTrainingSample(50.0);
}

void SelfplayThread::writeDataToFile()
{
  nnInputBuf.resize(NNINPUT_CHANNELS_V1 * param.sampleNumEachFile);
  nnOutputBuf.resize(NNOUTPUT_CHANNELS_V1 * param.sampleNumEachFile);
  static_assert(sizeof(ModelOutputPolicyV1) + sizeof(ModelOutputValueV1) == sizeof(float) * NNOUTPUT_CHANNELS_V1);
  
  for (int i = 0; i < param.sampleNumEachFile; i++)
  {
    memcpy(nnInputBuf.data() + i * NNINPUT_CHANNELS_V1, sampleData[i].nnInputVector, sizeof(float) * NNINPUT_CHANNELS_V1);
    memcpy(nnOutputBuf.data() + i * NNOUTPUT_CHANNELS_V1, &sampleData[i].policyTarget, sizeof(ModelOutputPolicyV1));
    memcpy(nnOutputBuf.data() + i * NNOUTPUT_CHANNELS_V1 + NNOUTPUT_CHANNELS_POLICY_V1, &sampleData[i].valueTarget, sizeof(ModelOutputValueV1));
  }

  string outputPath = param.exportDataDir;
  if (outputPath.back() != '/' && outputPath.back() != '\\')
    outputPath = outputPath + "/";
  outputPath = outputPath + generateHexFileName(rand) + ".npz";

  cnpy::npz_save(outputPath, "x", nnInputBuf.data(), { uint64_t(param.sampleNumEachFile), NNINPUT_CHANNELS_V1 }, "w"); // "w" 表示写模式
  cnpy::npz_save(outputPath, "label", nnOutputBuf.data(), { uint64_t(param.sampleNumEachFile), NNOUTPUT_CHANNELS_V1 }, "a"); // "a" 表示追加模式

}
