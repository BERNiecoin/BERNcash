#include "miningpage.h"
#include "ui_miningpage.h"
//#include "addresstablemodel.h"
#include "wallet.h"
CWallet *wallet;
extern QString glbAddress;
MiningPage::MiningPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MiningPage)
{
    ui->setupUi(this);

    //setFixedSize(400, 420);

    minerActive = false;

    minerProcess = new QProcess(this);
    minerProcess->setProcessChannelMode(QProcess::MergedChannels);

    readTimer = new QTimer(this);

    acceptedShares = 0;
    rejectedShares = 0;

    roundAcceptedShares = 0;
    roundRejectedShares = 0;

    initThreads = 0;

    connect(readTimer, SIGNAL(timeout()), this, SLOT(readProcessOutput()));

    connect(ui->startButton, SIGNAL(pressed()), this, SLOT(startPressed()));
    connect(ui->typeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
    connect(ui->debugCheckBox, SIGNAL(toggled(bool)), this, SLOT(debugToggled(bool)));
    connect(minerProcess, SIGNAL(started()), this, SLOT(minerStarted()));
    connect(minerProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(minerError(QProcess::ProcessError)));
    connect(minerProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(minerFinished()));
    connect(minerProcess, SIGNAL(readyRead()), this, SLOT(readProcessOutput()));
}

MiningPage::~MiningPage()
{
    minerProcess->kill();
    delete ui;
}

void MiningPage::setModel(ClientModel *model)
{
    this->model = model;
    loadSettings();

    bool pool = model->getMiningType() == ClientModel::PoolMining;
    ui->threadsBox->setValue(4);
    ui->scantimeBox->setValue(10);
    ui->typeBox->setCurrentIndex(pool ? 2 : 0);
    MiningPage::typeChanged(0); // init p2p values
//    if (model->getMiningStarted())
//        startPressed();
}

void MiningPage::startPressed()
{
    initThreads = ui->threadsBox->value();

    if (minerActive == false)
    {
        saveSettings();

        if (getMiningType() == ClientModel::SoloMining)
            minerStarted();
        else
            startPoolMining();
    }
    else
    {
        if (getMiningType() == ClientModel::SoloMining)
        {
            minerFinished();
            enablePoolMiningControls(false);
        }
        else
            stopPoolMining();
    }
}

void MiningPage::startPoolMining()
{
    QStringList args;
    QString url = ui->serverLine->text();
   // if (!url.contains("http://"))
   //     url.prepend("http://");
    QString urlLine = QString("%1:%2").arg(url, ui->portLine->text());
    QString userpassLine = QString("%1:%2").arg(ui->usernameLine->text(), ui->passwordLine->text());
    args << "--algo" << "x14";
    args << "--scantime" << ui->scantimeBox->text().toAscii();
    args << "--url" << urlLine.toAscii();
    args << "--userpass" << userpassLine.toAscii();
    args << "--threads" << ui->threadsBox->text().toAscii();
    args << "--retries" << "-1"; // Retry forever.
    args << "-P"; // This is needed for this to work correctly on Windows. Extra protocol dump helps flush the buffer quicker.

    threadSpeed.clear();

    acceptedShares = 0;
    rejectedShares = 0;

    roundAcceptedShares = 0;
    roundRejectedShares = 0;

    // If minerd is in current path, then use that. Otherwise, assume minerd is in the path somewhere.
    QString program = QDir::current().filePath("minerd");
    if (!QFile::exists(program))
        program = "minerd";

    if (ui->debugCheckBox->isChecked())
        ui->list->addItem(args.join(" ").prepend(" ").prepend(program));

    ui->mineSpeedLabel->setText("Speed: N/A");
    ui->shareCount->setText("Accepted: 0 - Rejected: 0");
    minerProcess->start(program,args);
    minerProcess->waitForStarted(-1);

    readTimer->start(500);
}

void MiningPage::stopPoolMining()
{
    ui->mineSpeedLabel->setText("");
    minerProcess->kill();
    readTimer->stop();
}

void MiningPage::saveSettings()
{
    model->setMiningDebug(ui->debugCheckBox->isChecked());
    model->setMiningScanTime(ui->scantimeBox->value());
    model->setMiningServer(ui->serverLine->text());
    model->setMiningPort(ui->portLine->text());
    model->setMiningUsername(ui->usernameLine->text());
    model->setMiningPassword(ui->passwordLine->text());
}

void MiningPage::loadSettings()
{
    ui->debugCheckBox->setChecked(model->getMiningDebug());
    ui->scantimeBox->setValue(model->getMiningScanTime());
    ui->serverLine->setText(model->getMiningServer());
    ui->portLine->setText(model->getMiningPort());
    ui->usernameLine->setText(model->getMiningUsername());
    ui->passwordLine->setText(model->getMiningPassword());
}

void MiningPage::readProcessOutput()
{
    QByteArray output;

    minerProcess->reset();

    output = minerProcess->readAll();

    QString outputString(output);

    if (!outputString.isEmpty())
    {
        QStringList list = outputString.split("\n", QString::SkipEmptyParts);
        int i;
        for (i=0; i<list.size(); i++)
        {
            QString line = list.at(i);

            // Ignore protocol dump
            if (!line.startsWith("[") || line.contains("JSON protocol") || line.contains("HTTP hdr") || line.contains("params") || line.contains("error"))
                continue;

            if (ui->debugCheckBox->isChecked())
            {
                line.remove("[0m");
                line.remove("[36m");
                line.remove("[33m");
                line.remove("[31m");
                ui->list->addItem(line.trimmed());
                ui->list->scrollToBottom();
            }

            if (line.contains("accepted:"))
                reportToList("Share accepted! Woo Hoo! Feel The BERN!!!", SHARE_SUCCESS, getTime(line));
            else if (line.contains("rejected"))
                reportToList("Share rejected. No worries, it happens.", SHARE_FAIL, getTime(line));
            else if (line.contains(" x14 block"))
                reportToList("New block detected on the BERN network. Beginning work on the new block.", LONGPOLL, getTime(line));
            else if (line.contains("Supported options:"))
                reportToList("Miner didn't start properly. Try checking your settings.", ERROR, NULL);
            else if (line.contains("The requested URL returned error: 403"))
                reportToList("Couldn't connect. Please check your username and password.", ERROR, NULL);
            else if (line.contains("HTTP request failed"))
                reportToList("Couldn't connect. Please check pool server and port.", ERROR, NULL);
            else if (line.contains("JSON-RPC call failed"))
                reportToList("Couldn't communicate with server. Retrying in 30 seconds.", ERROR, NULL);
            else if (line.contains("CPU") && line.contains("kH/s"))
            {
                // QString threadIDstr = line.at(line.indexOf("CPU")+5);
                QString threadIDstr = line.mid(line.indexOf("CPU")+5,2);
                threadIDstr.remove(":");
                int threadID = threadIDstr.toInt();
                    //reportToList("threadID->", ERROR, NULL);
                   // reportToList(threadIDstr, ERROR, NULL);
                int threadSpeedindx = line.indexOf("#")+3;
                QString threadSpeedstr = line.mid(threadSpeedindx,7);
                //threadSpeedstr.chop(8);
                //threadSpeedstr= threadSpeedstr.at(line.indexOf(":"));
                threadSpeedstr.remove(":");
                threadSpeedstr.remove(" ");
                threadSpeedstr.remove("k");
                //threadSpeedstr.remove('\n');


                //reportToList(threadSpeedstr, ERROR, NULL);
                double speed=0;
                speed = threadSpeedstr.toDouble();

                threadSpeed[threadID] = speed;

                updateSpeed();
            }
        }
    }
}

void MiningPage::minerError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart)
    {
        reportToList("Miner failed to start. Make sure you have the minerd executable and any associated files in the same directory as BERN-qt.", ERROR, NULL);
    }
}

void MiningPage::minerFinished()
{
    if (getMiningType() == ClientModel::SoloMining)
        reportToList("Solo mining stopped.", ERROR, NULL);
    else
        reportToList("Miner exited.", ERROR, NULL);
    ui->list->addItem("");
    minerActive = false;
    resetMiningButton();
    model->setMining(getMiningType(), false, initThreads, 0);
}

void MiningPage::minerStarted()
{
    if (!minerActive)
        if (getMiningType() == ClientModel::SoloMining)
            reportToList("Solo mining started.", ERROR, NULL);
        else
            reportToList("Miner started. You might not see any output for a few minutes.", STARTED, NULL);
    minerActive = true;
    resetMiningButton();
    model->setMining(getMiningType(), true, initThreads, 0);
}

void MiningPage::updateSpeed()
{
    double totalSpeed=0;
    int totalThreads=0;

    QMapIterator<int, double> iter(threadSpeed);
    while(iter.hasNext())
    {
        iter.next();
        totalSpeed += iter.value();
        totalThreads++;
    }

    // If all threads haven't reported the hash speed yet, make an assumption
    if (totalThreads != initThreads)
    {
        totalSpeed = (totalSpeed/totalThreads)*initThreads;
    }

    QString speedString = QString("%1").arg(totalSpeed);
    QString threadsString = QString("%1").arg(initThreads);

    QString acceptedString = QString("%1").arg(acceptedShares);
    QString rejectedString = QString("%1").arg(rejectedShares);

    QString roundAcceptedString = QString("%1").arg(roundAcceptedShares);
    QString roundRejectedString = QString("%1").arg(roundRejectedShares);

    if (totalThreads == initThreads)
        ui->mineSpeedLabel->setText(QString("Speed: %1 khash/sec - %2 thread(s)").arg(speedString, threadsString));
    else
        ui->mineSpeedLabel->setText(QString("Speed: ~%1 khash/sec - %2 thread(s)").arg(speedString, threadsString));

    ui->shareCount->setText(QString("Accepted: %1 (%3) - Rejected: %2 (%4)").arg(acceptedString, rejectedString, roundAcceptedString, roundRejectedString));

    model->setMining(getMiningType(), true, initThreads, totalSpeed*1000);
}

void MiningPage::reportToList(QString msg, int type, QString time)
{
    QString message;
    if (time == NULL)
        message = QString("[%1] - %2").arg(QTime::currentTime().toString(), msg);
    else
        message = QString("[%1] - %2").arg(time, msg);

    switch(type)
    {
        case SHARE_SUCCESS:
            acceptedShares++;
            roundAcceptedShares++;
            updateSpeed();
            break;

        case SHARE_FAIL:
            rejectedShares++;
            roundRejectedShares++;
            updateSpeed();
            break;

        case LONGPOLL:
            roundAcceptedShares = 0;
            roundRejectedShares = 0;
            break;

        default:
            break;
    }

    ui->list->addItem(message);
    ui->list->scrollToBottom();
}

// Function for fetching the time
QString MiningPage::getTime(QString time)
{
    if (time.contains("["))
    {
        time.resize(21);
        time.remove("[");
        time.remove("]");
        //time.remove(0,11);

        return time;
    }
    else
        return NULL;
}

void MiningPage::enableMiningControls(bool enable)
{
    ui->typeBox->setEnabled(enable);
    ui->threadsBox->setEnabled(enable);
    if (enable) ui->threadsBox->setValue(4);
    ui->scantimeBox->setEnabled(enable);
    if (enable) ui->scantimeBox->setValue(10);
    ui->serverLine->setEnabled(enable);
    ui->portLine->setEnabled(enable);
    ui->usernameLine->setEnabled(enable);
    ui->passwordLine->setEnabled(enable);
}

void MiningPage::enablePoolMiningControls(bool enable)
{
    ui->scantimeBox->setEnabled(enable);
    ui->serverLine->setEnabled(enable);
    ui->portLine->setEnabled(enable);
    ui->usernameLine->setEnabled(enable);
    ui->passwordLine->setEnabled(enable);
}

ClientModel::MiningType MiningPage::getMiningType()
{
    if (ui->typeBox->currentIndex() == 0)  // Easy Mining
    {
        return ClientModel::EasyMining;
    }
    else if (ui->typeBox->currentIndex() == 1)  // Solo Mining
    {
        return ClientModel::SoloMining;
    }
    else if (ui->typeBox->currentIndex() == 2)  // Pool Mining
    {
        return ClientModel::PoolMining;
    }
    return ClientModel::SoloMining;
}

void MiningPage::typeChanged(int index)
{
    if (index == 0)  // Easy Mining
    {
        enablePoolMiningControls(false);
        ui->passwordLine->setText("x");
        ui->serverLine->setText("stratum+tcp://yiimp.ccminer.org");
        ui->portLine->setText("3933");
        ui->usernameLine->setText(glbAddress);
        //}
    }
    else if (index == 1)  // Solo Mining
    {
        enablePoolMiningControls(false);
    }
    else if (index == 2)  // Pool Mining
    {
        enablePoolMiningControls(true);
    }
}

void MiningPage::debugToggled(bool checked)
{
    model->setMiningDebug(checked);
}

void MiningPage::resetMiningButton()
{
    ui->startButton->setText(minerActive ? "Stop Mining" : "Start Mining");
    enableMiningControls(!minerActive);
}
