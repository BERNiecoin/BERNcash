#include "statisticspage.h"
#include "ui_statisticspage.h"
#include "main.h"
#include "wallet.h"
#include "init.h"
#include "base58.h"
#include "clientmodel.h"
#include "bitcoinrpc.h"
#include <sstream>
#include <string>

using namespace json_spirit;

StatisticsPage::StatisticsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatisticsPage)
{
    ui->setupUi(this);
    
    resize(400, 420);
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerCountDown()));
    timer->start(1000);
}

int heightPrevious = -1;
int connectionPrevious = -1;
int volumePrevious = -1;
double rewardPrevious = -1;
double netPawratePrevious = -1;
double pawratePrevious = -1;
double hardnessPrevious = -1;
double hardnessPrevious2 = -1;
int stakeminPrevious = -1;
int stakemaxPrevious = -1;
QString stakecPrevious = "";
int dec1 = 10;


void StatisticsPage::updateStatistics()
{
    double pHardness = GetDifficulty();
    double pHardness2 = GetDifficulty(GetLastBlockIndex(pindexBest, true));
    double pPawrate2 = 0.000;
    int nHeight = pindexBest->nHeight;
    int pPawrate = getBlockHashrate(nHeight);
    double nSubsidy;
    if (pindexBest->nHeight > 6519080)
        nSubsidy = 0;
    else if (pindexBest->nHeight > 4927680)
        nSubsidy = 10;
    else if (pindexBest->nHeight > 3875760)
        nSubsidy = 100;
    else if (pindexBest->nHeight > 720000)
        nSubsidy = 10;
    else
        nSubsidy = 250;

    uint64 nMinWeight = 0, nMaxWeight = 0, nWeight = 0;
    pwalletMain->GetStakeWeight(*pwalletMain, nMinWeight, nMaxWeight, nWeight);
    uint64_t nNetworkWeight = GetPoSKernelPS();
    int64_t volume = ((pindexBest->nMoneySupply)/1000000);
    int peers = this->model->getNumConnections();
    pPawrate2 = (double)abs(pPawrate);
    QString height = QString::number(nHeight);
    QString stakemin = QString::number(nMinWeight);
    QString stakemax = QString::number(nNetworkWeight);
    QString subsidy = QString::number(nSubsidy, 'f', 0);
    QString hardness = QString::number(pHardness, 'f', 6);
    QString hardness2 = QString::number(pHardness2, 'f', 6);
    QString pawrate = QString::number((pPawrate2/ 1000), 'f', 3);
    QString Qlpawrate = model->getLastBlockDate().toString();
    QString QPeers = QString::number(peers);
    QString qVolume = QString::number(volume);

    if(nHeight > heightPrevious)
    {
        ui->heightBox->setText("<b><font color=\"green\">" + height + "</font></b>");
    } else {
    ui->heightBox->setText(height);
    }

    if((int)nMinWeight > stakeminPrevious)
    {
        ui->minBox->setText("<b><font color=\"green\">" + stakemin + "</font></b>");
    } else if((int)nMinWeight < stakeminPrevious) {
        ui->minBox->setText("<b><font color=\"red\">" + stakemin + "</font></b>");
    } else {
    ui->minBox->setText(stakemin);
    }

    if((int)nNetworkWeight > stakemaxPrevious)
    {
        ui->maxBox->setText("<b><font color=\"green\">" + stakemax + "</font></b>");
    } else if((int)nNetworkWeight < stakemaxPrevious) {
        ui->maxBox->setText("<b><font color=\"red\">" +  stakemax + "</font></b>");
    } else {
    ui->maxBox->setText(stakemax);
    }

    if(nSubsidy < rewardPrevious)
    {
        ui->rewardBox->setText("<b><font color=\"red\">" + subsidy + " BERN</font></b>");
    } else if(nSubsidy > rewardPrevious) {
        ui->rewardBox->setText("<b><font color=\"green\">" +  subsidy + " BERN</font></b>");
    } else {
    ui->rewardBox->setText(subsidy+" BERN");
    }
    
    if(pHardness > hardnessPrevious)
    {
        ui->diffBox->setText("<b><font color=\"green\">" + hardness + "</font></b>");
    } else if(pHardness < hardnessPrevious) {
        ui->diffBox->setText("<b><font color=\"red\">" + hardness + "</font></b>");
    } else {
        ui->diffBox->setText(hardness);        
    }

    if(pHardness2 > hardnessPrevious2)
    {
        ui->diffBox2->setText("<b><font color=\"green\">" + hardness2 + "</font></b>");
    } else if(pHardness2 < hardnessPrevious2) {
        ui->diffBox2->setText("<b><font color=\"red\">" + hardness2 + "</font></b>");
    } else {
        ui->diffBox2->setText(hardness2);
    }
    
    if(pPawrate2 > netPawratePrevious)
    {
        ui->pawrateBox->setText("<b><font color=\"green\">" + pawrate + " Hash/sec</font></b>");
    } else if(pPawrate2 < netPawratePrevious) {
        ui->pawrateBox->setText("<b><font color=\"red\">" + pawrate + " Hash/sec</font></b>");
    } else {
        ui->pawrateBox->setText(pawrate + " Hash/sec");
    }

    if(Qlpawrate != pawratePrevious)
    {
        ui->localBox->setText("<b><font color=\"green\">" + Qlpawrate + "</font></b>");
    } else {
    ui->localBox->setText(Qlpawrate);
    }
    
    if(peers > connectionPrevious)
    {
        ui->connectionBox->setText("<b><font color=\"green\">" + QPeers + "</font></b>");
    } else if(peers < connectionPrevious) {
        ui->connectionBox->setText("<b><font color=\"red\">" + QPeers + "</font></b>");
    } else {
        ui->connectionBox->setText(QPeers);  
    }

    if(volume > volumePrevious)
    {
        ui->volumeBox->setText("<b><font color=\"green\">" + qVolume + " BERN" + "</font></b>");
    } else if(volume < volumePrevious) {
        ui->volumeBox->setText("<b><font color=\"red\">" + qVolume + " BERN" + "</font></b>");
    } else {
        ui->volumeBox->setText(qVolume + " BERN");
    }
    updatePrevious(nHeight, nMinWeight, nNetworkWeight, nSubsidy, pHardness, pHardness2, pPawrate2, Qlpawrate, peers, volume);

    if (pindexBest->nHeight < 7200001)
    {
        int daysLeft =(720000 - nHeight) / (1440 * 2);
        ui->phaselbl1->setText(QString("<b>Block 2 -> 720000 : PoW (250 BERN) & PoS</b><br/>Approximately %1 days remain." ).arg(daysLeft));

    }
    else
        ui->phaselbl1->setText("Block 2 -> 720000 : PoW (250 BERN) & PoS");
    if (pindexBest->nHeight > 720000 && pindexBest->nHeight < 3875761)
    {
        int daysLeft =((3875760 - 720000) - (nHeight - 720000)) / (1440 * 2);
        ui->phaselbl2->setText(QString("<b>Block 7200001 -> 3875760 : PoW (10 BERN)  & PoS</b><br/>Approximately %1 days remain." ).arg(daysLeft));
    }
    else
        ui->phaselbl2->setText("Block 7200001 -> 3875760 : PoW (10 BERN)  & PoS");
    if (pindexBest->nHeight > 3875760 && pindexBest->nHeight < 4927681)
    {
        int daysLeft =((4927680 - 3875760) - (nHeight - 3875760)) / (1440 * 2);
        ui->phaselbl3->setText(QString("<b>Block 3875761 -> 4927680 : PoW (100 BERN) & PoS</b><br/>Approximately %1 days remain." ).arg(daysLeft));
    }
    else
        ui->phaselbl3->setText("Block 3875761 -> 4927680 : PoW (100 BERN) & PoS");
    if (pindexBest->nHeight > 4927680 && pindexBest->nHeight < 6519081)
    {
        int daysLeft =((6519080 - 4927680) - (nHeight - 4927681)) / (1440 * 2);
        ui->phaselbl4->setText(QString("<b>Block 4927681 -> 6519080 : PoW (10 BERN)  & PoS</b><br/>Approximately %1 days remain." ).arg(daysLeft));
    }
    else
        ui->phaselbl4->setText("Block 4927681 -> 6519080 : PoW (10 BERN)  & PoS");
    if (pindexBest->nHeight > 6519080)
        ui->phaselbl5->setText("<b>Block 6519081 -> ...  :  Full PoS</b>");
    else
        ui->phaselbl5->setText("Block 6519081 -> ...  :  Full PoS");
}

void StatisticsPage::updatePrevious(int nHeight, int nMinWeight, int nNetworkWeight, double nSubsidy, double pHardness, double pHardness2, double pPawrate2, QString Qlpawrate, int peers, int volume)
{
    heightPrevious = nHeight;
    stakeminPrevious = nMinWeight;
    stakemaxPrevious = nNetworkWeight;
    rewardPrevious = nSubsidy;
    hardnessPrevious = pHardness;
    hardnessPrevious2 = pHardness2;
    netPawratePrevious = pPawrate2;
    pawratePrevious = Qlpawrate;
    connectionPrevious = peers;
    volumePrevious = volume;
}

void StatisticsPage::setModel(ClientModel *model)
{
    updateStatistics();
    this->model = model;
}

StatisticsPage::~StatisticsPage()
{
    delete ui;
}

void StatisticsPage::timerCountDown()
{
    dec1 = dec1 - 1;
    ui->startButton->setText("Update Statistics " + QString::number(dec1));
    if (dec1 == 0)
    {
        updateStatistics();
        dec1 = 10;
    }
}

void StatisticsPage::on_startButton_released()
{
    updateStatistics();
    dec1 = 11;
}
