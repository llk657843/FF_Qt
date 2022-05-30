#include "record_setting_form.h"
#include "ui_record_setting_form.h"
#include "../player_controller/encoder_controller.h"
#include <QtWidgets/qfiledialog.h>
#include <warning.h>
#include "qmessagebox.h"
RecordSettingForm::RecordSettingForm(QWidget* parent) :QWidget(parent),ui(new Ui::RecordSettingFormUI)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);
	OnModifyUI();
}

RecordSettingForm::~RecordSettingForm()
{
	delete ui;
	ui = nullptr;
}

void RecordSettingForm::OnModifyUI()
{
	setFixedSize(420, 252);
	connect(ui->btn_start_record,&QPushButton::clicked,this,&RecordSettingForm::SlotStartClicked);
	connect(ui->btn_file_pos,&QPushButton::clicked,this,&RecordSettingForm::SlotFilePositionClicked);
	ui->bitrate_combo->addItem(QString::number(4000000));
	ui->bitrate_combo->addItem(QString::number(1500000));
	ui->framerate_combo->addItem(QString::number(25));
	ui->framerate_combo->addItem(QString::number(30));
	ui->pix_rate_combo->addItem(QString("1920*1080"));
	ui->pix_rate_combo->addItem(QString("1280*720"));
	ui->pix_rate_combo->addItem(QString("640*480"));
	ui->line_file_pos->setText(QString("D:/record.mp4"));
}

void RecordSettingForm::SlotStartClicked()
{
	EncoderController::GetInstance()->SetBitrate(ui->bitrate_combo->currentText().toInt());
	EncoderController::GetInstance()->SetFramerate(ui->framerate_combo->currentText().toInt());
	if (ui->pix_rate_combo->currentText() == "1920*1080") 
	{
		EncoderController::GetInstance()->SetPixRate(1920,1080);
	}
	else if (ui->pix_rate_combo->currentText() == "1280*720") 
	{
		EncoderController::GetInstance()->SetPixRate(1280, 720);
	}
	else if(ui->pix_rate_combo->currentText() == "640*480")
	{
		EncoderController::GetInstance()->SetPixRate(640, 480);
	}
	QString file_path = ui->line_file_pos->text();
	if(file_path.isEmpty() || file_path.trimmed().isEmpty())
	{
		QMessageBox::warning(this, "warning", "file path is empty");
		return;
	}
	QFile file(file_path);
	bool b_open = file.open(QIODevice::ReadWrite);
	file.close();
	if (b_open) 
	{
		EncoderController::GetInstance()->SetFilePath(file_path);
		EncoderController::GetInstance()->ReadyEncode();
		EncoderController::GetInstance()->StartCatch();
		close();
	}
	else
	{
		QMessageBox::warning(this, "warning", "file open failed");
	}
	
}

void RecordSettingForm::SlotFilePositionClicked()
{
	QString name = QFileDialog::getSaveFileName();
	if (name.isEmpty() || name.trimmed().isEmpty()) 
	{
		return;
	}
	ui->line_file_pos->setText(name);
}
