#include <gtest/gtest.h>
#include <stdint.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

// Include the mocks first so they override subsequent includes.
#include "mock/Memory.h"

#include "Bytes.h"
#include "Audio/Spc700.h"
#include "Audio/Timer.h"

namespace Audio
{

const uint8_t A_VALUE = 0x12;
const uint8_t X_VALUE = 0x34;
const uint8_t Y_VALUE = 0x56;
const uint8_t P_VALUE = 0x00;
const uint8_t SP_VALUE = 0xFF;

const QString JSON_PATH = "./test_data/spc700/v1/";


class Spc700Test : public ::testing::Test
{
protected:
    Spc700Test();
    ~Spc700Test() override;

    void SetUp() override;
    void TearDown() override;

    void ResetState();
    void RunInstructionTest(const QString &opcodeName, const QString &opcode);
    void FormatData(const QJsonObject &obj, QString &str);

    Spc700 *cpu;
    Memory *memory;
    Timer *timer;
};


Spc700Test::Spc700Test()
{
    memory = new Memory();
    timer = new Timer(memory);
    cpu = new Spc700(memory, timer);
    memory->SetTimer(timer);
}

Spc700Test::~Spc700Test()
{
    delete cpu;
    delete timer;
    delete memory;
}

void Spc700Test::SetUp()
{
    ResetState();
}

void Spc700Test::TearDown()
{

}

void Spc700Test::ResetState()
{
    cpu->reg.a = A_VALUE;
    cpu->reg.x = X_VALUE;
    cpu->reg.y = Y_VALUE;
    cpu->reg.p = P_VALUE;
    cpu->reg.sp = SP_VALUE;
    cpu->reg.pc = 0;
}

void Spc700Test::RunInstructionTest(const QString &opcodeName, const QString &opcode)
{
    QString testName = opcodeName + ": ";

    QString filename = QStringLiteral("%1%2.json").arg(JSON_PATH).arg(opcode.toLower());
    QFile jsonFile(filename);
    ASSERT_TRUE(jsonFile.open(QIODevice::ReadOnly)) << qPrintable(testName + "Couldn't open " + filename);

    QJsonDocument json = QJsonDocument::fromJson(jsonFile.readAll());
    ASSERT_TRUE(json.isArray()) << qPrintable(testName);
    QJsonArray array = json.array();

    for (int i = 0; i < array.size(); i++)
    {
        QJsonObject obj = array[i].toObject();

        testName = opcodeName + ": " + obj["name"].toString();

        // Set register initial values.
        QJsonObject initial = obj["initial"].toObject();
        cpu->reg.a = initial["a"].toInt();
        cpu->reg.x = initial["x"].toInt();
        cpu->reg.y = initial["y"].toInt();
        cpu->reg.p = initial["psw"].toInt();
        cpu->reg.sp = initial["sp"].toInt();
        cpu->reg.pc = initial["pc"].toInt();

        // Set RAM initial values.
        QJsonArray initalRam = initial["ram"].toArray();
        for (int j = 0; j < initalRam.size(); j++)
        {
            QJsonArray pair = initalRam[j].toArray();
            int32_t addr = pair[0].toInt();
            int32_t val = pair[1].toInt();
            ASSERT_GE(addr, 0) << qPrintable(testName);
            ASSERT_LE(addr, 0xFFFF) << qPrintable(testName);
            ASSERT_GE(val, 0) << qPrintable(testName);
            ASSERT_LE(val, 0xFF) << qPrintable(testName);
            memory->Write8Bit(addr, val);
        }

        timer->ResetCounter();

        // Run the opcode.
        cpu->ProcessOpCode();

        // Verify result register values.
        QJsonObject final = obj["final"].toObject();
        EXPECT_EQ(cpu->reg.a, final["a"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.x, final["x"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.y, final["y"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.p, final["psw"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.sp, final["sp"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.pc, final["pc"].toInt()) << qPrintable(testName);

        // Verify result RAM values.
        QJsonArray finalRam = final["ram"].toArray();
        for (int j = 0; j < finalRam.size(); j++)
        {
            QJsonArray pair = finalRam[j].toArray();
            int32_t addr = pair[0].toInt();
            int32_t val = pair[1].toInt();
            ASSERT_GE(addr, 0) << qPrintable(testName);
            ASSERT_LE(addr, 0xFFFF) << qPrintable(testName);
            ASSERT_GE(val, 0) << qPrintable(testName);
            ASSERT_LE(val, 0xFF) << qPrintable(testName);
            EXPECT_EQ(memory->ReadRaw8Bit(addr), val) << qPrintable(testName);
        }

        // Verify timing
        QJsonArray cycles = obj["cycles"].toArray();
        ASSERT_EQ(timer->GetCounter(), cycles.count()) << qPrintable(testName);

        // If there were errors, stop processing this opcode. We don't want 10000 errors.
        if (this->HasNonfatalFailure())
        {
            QString str;
            this->FormatData(obj, str);
            qDebug("Encountered failure, not continuing.");
            qDebug("Test Data: %s", qPrintable(str));
            return;
        }
    }
}

void Spc700Test::FormatData(const QJsonObject &obj, QString &str)
{
    str += "\nname: \"" + obj["name"].toString() + "\"\n";

    QJsonObject initial = obj["initial"].toObject();
    str += "initial: \n";
    str += QStringLiteral("\ta: 0x%1\n").arg(initial["a"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tx: 0x%1\n").arg(initial["x"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\ty: 0x%1\n").arg(initial["y"].toInt(), 4, 16, QChar('0'));
    uint8_t p = initial["psw"].toInt();
    str += QStringLiteral("\tp: 0x%1 (n=%2 v=%3 p=%4 b=%5 h=%6 i=%7 z=%8 c=%9)\n")
        .arg(p, 2, 16, QChar('0'))
        .arg(p >> 7)
        .arg((p >> 6) & 0x01)
        .arg((p >> 5) & 0x01)
        .arg((p >> 4) & 0x01)
        .arg((p >> 3) & 0x01)
        .arg((p >> 2) & 0x01)
        .arg((p >> 1) & 0x01)
        .arg(p & 0x01);
    str += QStringLiteral("\tsp: 0x%1\n").arg(initial["sp"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tpc: 0x%1\n").arg(initial["pc"].toInt(), 4, 16, QChar('0'));

    QJsonArray initalRam = initial["ram"].toArray();
    str += "\tram: \n";
    QStringList initialRamStrings;
    for (int i = 0; i < initalRam.size(); i++)
    {
        QJsonArray pair = initalRam[i].toArray();
        initialRamStrings.append(QStringLiteral("\t\t0x%1 = 0x%2").arg(pair[0].toInt(), 6, 16, QChar('0')).arg(pair[1].toInt(), 2, 16, QChar('0')));
    }
    initialRamStrings.sort();
    str += initialRamStrings.join(QChar('\n')) + "\n";

    QJsonObject final = obj["final"].toObject();
    str += "expected: \n";
    str += QStringLiteral("\ta: 0x%1\n").arg(final["a"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tx: 0x%1\n").arg(final["x"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\ty: 0x%1\n").arg(final["y"].toInt(), 4, 16, QChar('0'));
    p = final["psw"].toInt();
    str += QStringLiteral("\tp: 0x%1 (n=%2 v=%3 p=%4 b=%5 h=%6 i=%7 z=%8 c=%9)\n")
        .arg(p, 2, 16, QChar('0'))
        .arg(p >> 7)
        .arg((p >> 6) & 0x01)
        .arg((p >> 5) & 0x01)
        .arg((p >> 4) & 0x01)
        .arg((p >> 3) & 0x01)
        .arg((p >> 2) & 0x01)
        .arg((p >> 1) & 0x01)
        .arg(p & 0x01);
    str += QStringLiteral("\tsp: 0x%1\n").arg(final["sp"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tpc: 0x%1\n").arg(final["pc"].toInt(), 4, 16, QChar('0'));

    QJsonArray finalRam = final["ram"].toArray();
    str += "\tram: \n";
    QStringList finalRamStrings;
    for (int i = 0; i < finalRam.size(); i++)
    {
        QJsonArray pair = finalRam[i].toArray();
        finalRamStrings.append(QStringLiteral("\t\t0x%1 = 0x%2").arg(pair[0].toInt(), 6, 16, QChar('0')).arg(pair[1].toInt(), 2, 16, QChar('0')));
    }
    finalRamStrings.sort();
    str += finalRamStrings.join(QChar('\n')) + "\n";

    str += "actual: \n";
    str += QStringLiteral("\ta: 0x%1\n").arg(cpu->reg.a, 4, 16, QChar('0'));
    str += QStringLiteral("\tx: 0x%1\n").arg(cpu->reg.x, 4, 16, QChar('0'));
    str += QStringLiteral("\ty: 0x%1\n").arg(cpu->reg.y, 4, 16, QChar('0'));
    p = cpu->reg.p;
    str += QStringLiteral("\tp: 0x%1 (n=%2 v=%3 p=%4 b=%5 h=%6 i=%7 z=%8 c=%9))\n")
        .arg(p, 2, 16, QChar('0'))
        .arg(p >> 7)
        .arg((p >> 6) & 0x01)
        .arg((p >> 5) & 0x01)
        .arg((p >> 4) & 0x01)
        .arg((p >> 3) & 0x01)
        .arg((p >> 2) & 0x01)
        .arg((p >> 1) & 0x01)
        .arg(p & 0x01);
    str += QStringLiteral("\tsp: 0x%1\n").arg(cpu->reg.sp, 4, 16, QChar('0'));
    str += QStringLiteral("\tpc: 0x%1\n").arg(cpu->reg.pc, 4, 16, QChar('0'));

    str += "\tram: \n";
    QStringList expectedRamStrings;
    for (int i = 0; i < finalRam.size(); i++)
    {
        QJsonArray pair = finalRam[i].toArray();
        expectedRamStrings.append(QStringLiteral("\t\t0x%1 = 0x%2").arg(pair[0].toInt(), 6, 16, QChar('0')).arg(memory->Read8Bit(pair[0].toInt()), 2, 16, QChar('0')));
    }
    expectedRamStrings.sort();
    str += expectedRamStrings.join(QChar('\n')) + "\n";
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MOV_A_IndirectXp)
{
    this->RunInstructionTest(this->test_info_->name(), "BF");
}

TEST_F(Spc700Test, TEST_MOV_A_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "E4");
}

TEST_F(Spc700Test, TEST_MOV_A_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "E5");
}

TEST_F(Spc700Test, TEST_MOV_A_IndirectX)
{
    this->RunInstructionTest(this->test_info_->name(), "E6");
}

TEST_F(Spc700Test, TEST_MOV_A_IndirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "E7");
}

TEST_F(Spc700Test, TEST_MOV_A_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "E8");
}

TEST_F(Spc700Test, TEST_MOV_A_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "F4");
}

TEST_F(Spc700Test, TEST_MOV_A_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "F5");
}

TEST_F(Spc700Test, TEST_MOV_A_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "F6");
}

TEST_F(Spc700Test, TEST_MOV_A_IndirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "F7");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MOV_X_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "CD");
}

TEST_F(Spc700Test, TEST_MOV_X_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "E9");
}

TEST_F(Spc700Test, TEST_MOV_X_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "F8");
}

TEST_F(Spc700Test, TEST_MOV_X_DirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "F9");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MOV_Y_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "8D");
}

TEST_F(Spc700Test, TEST_MOV_Y_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "EB");
}

TEST_F(Spc700Test, TEST_MOV_Y_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "EC");
}

TEST_F(Spc700Test, TEST_MOV_Y_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "FB");
}

TEST_F(Spc700Test, TEST_MOVW_YA_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "BA");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MOV_IndirectXp_A)
{
    this->RunInstructionTest(this->test_info_->name(), "AF");
}

TEST_F(Spc700Test, TEST_MOV_Direct_A)
{
    this->RunInstructionTest(this->test_info_->name(), "C4");
}

TEST_F(Spc700Test, TEST_MOV_Absolute_A)
{
    this->RunInstructionTest(this->test_info_->name(), "C5");
}

TEST_F(Spc700Test, TEST_MOV_IndirectX_A)
{
    this->RunInstructionTest(this->test_info_->name(), "C6");
}

TEST_F(Spc700Test, TEST_MOV_IndirectIndexedX_A)
{
    this->RunInstructionTest(this->test_info_->name(), "C7");
}

TEST_F(Spc700Test, TEST_MOV_DirectIndexedX_A)
{
    this->RunInstructionTest(this->test_info_->name(), "D4");
}

TEST_F(Spc700Test, TEST_MOV_AbsoluteIndexedX_A)
{
    this->RunInstructionTest(this->test_info_->name(), "D5");
}

TEST_F(Spc700Test, TEST_MOV_AbsoluteIndexedY_A)
{
    this->RunInstructionTest(this->test_info_->name(), "D6");
}

TEST_F(Spc700Test, TEST_MOV_IndirectIndexedY_A)
{
    this->RunInstructionTest(this->test_info_->name(), "D7");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MOV_Absolute_X)
{
    this->RunInstructionTest(this->test_info_->name(), "C9");
}

TEST_F(Spc700Test, TEST_MOV_Direct_X)
{
    this->RunInstructionTest(this->test_info_->name(), "D8");
}

TEST_F(Spc700Test, TEST_MOV_DirectIndexedY_X)
{
    this->RunInstructionTest(this->test_info_->name(), "D9");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MOV_Direct_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "CB");
}

TEST_F(Spc700Test, TEST_MOV_Absolute_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "CC");
}

TEST_F(Spc700Test, TEST_MOV_DirectIndexedX_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "DB");
}

TEST_F(Spc700Test, TEST_MOVW_Direct_YA)
{
    this->RunInstructionTest(this->test_info_->name(), "DA");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MOV_A_X)
{
    this->RunInstructionTest(this->test_info_->name(), "7D");
}

TEST_F(Spc700Test, TEST_MOV_A_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "DD");
}

TEST_F(Spc700Test, TEST_MOV_X_A)
{
    this->RunInstructionTest(this->test_info_->name(), "5D");
}

TEST_F(Spc700Test, TEST_MOV_Y_A)
{
    this->RunInstructionTest(this->test_info_->name(), "FD");
}

TEST_F(Spc700Test, TEST_MOV_X_SP)
{
    this->RunInstructionTest(this->test_info_->name(), "9D");
}

TEST_F(Spc700Test, TEST_MOV_SP_X)
{
    this->RunInstructionTest(this->test_info_->name(), "BD");
}

TEST_F(Spc700Test, TEST_MOV_DP_DP)
{
    this->RunInstructionTest(this->test_info_->name(), "FA");
}

TEST_F(Spc700Test, TEST_MOV_DP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "8F");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_ADC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "84");
}

TEST_F(Spc700Test, TEST_ADC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "85");
}

TEST_F(Spc700Test, TEST_ADC_IndirectX)
{
    this->RunInstructionTest(this->test_info_->name(), "86");
}

TEST_F(Spc700Test, TEST_ADC_IndirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "87");
}

TEST_F(Spc700Test, TEST_ADC_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "88");
}

TEST_F(Spc700Test, TEST_ADC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "94");
}

TEST_F(Spc700Test, TEST_ADC_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "95");
}

TEST_F(Spc700Test, TEST_ADC_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "96");
}

TEST_F(Spc700Test, TEST_ADC_IndirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "97");
}

TEST_F(Spc700Test, TEST_ADC_IndirectToIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "99");
}

TEST_F(Spc700Test, TEST_ADC_DP_DP)
{
    this->RunInstructionTest(this->test_info_->name(), "89");
}

TEST_F(Spc700Test, TEST_ADC_DP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "98");
}

TEST_F(Spc700Test, TEST_ADDW_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "7A");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_SBC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "A4");
}

TEST_F(Spc700Test, TEST_SBC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "A5");
}

TEST_F(Spc700Test, TEST_SBC_IndirectX)
{
    this->RunInstructionTest(this->test_info_->name(), "A6");
}

TEST_F(Spc700Test, TEST_SBC_IndirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "A7");
}

TEST_F(Spc700Test, TEST_SBC_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "A8");
}

TEST_F(Spc700Test, TEST_SBC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "B4");
}

TEST_F(Spc700Test, TEST_SBC_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "B5");
}

TEST_F(Spc700Test, TEST_SBC_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "B6");
}

TEST_F(Spc700Test, TEST_SBC_IndirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "B7");
}

TEST_F(Spc700Test, TEST_SBC_IndirectToIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "B9");
}

TEST_F(Spc700Test, TEST_SBC_DP_DP)
{
    this->RunInstructionTest(this->test_info_->name(), "A9");
}

TEST_F(Spc700Test, TEST_SBC_DP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "B8");
}

TEST_F(Spc700Test, TEST_SUBW_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "9A");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_CMP_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "64");
}

TEST_F(Spc700Test, TEST_CMP_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "65");
}

TEST_F(Spc700Test, TEST_CMP_IndirectX)
{
    this->RunInstructionTest(this->test_info_->name(), "66");
}

TEST_F(Spc700Test, TEST_CMP_IndirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "67");
}

TEST_F(Spc700Test, TEST_CMP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "68");
}

TEST_F(Spc700Test, TEST_CMP_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "74");
}

TEST_F(Spc700Test, TEST_CMP_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "75");
}

TEST_F(Spc700Test, TEST_CMP_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "76");
}

TEST_F(Spc700Test, TEST_CMP_IndirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "77");
}

TEST_F(Spc700Test, TEST_CMP_IndirectToIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "79");
}

TEST_F(Spc700Test, TEST_CMP_DP_DP)
{
    this->RunInstructionTest(this->test_info_->name(), "69");
}

TEST_F(Spc700Test, TEST_CMP_DP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "78");
}

TEST_F(Spc700Test, TEST_CMP_X_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "C8");
}

TEST_F(Spc700Test, TEST_CMP_X_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "3E");
}

TEST_F(Spc700Test, TEST_CMP_X_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "1E");
}

TEST_F(Spc700Test, TEST_CMP_Y_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "AD");
}

TEST_F(Spc700Test, TEST_CMP_Y_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "7E");
}

TEST_F(Spc700Test, TEST_CMP_Y_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "5E");
}

TEST_F(Spc700Test, TEST_CMPW_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "5A");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_MUL_YA)
{
    this->RunInstructionTest(this->test_info_->name(), "CF");
}

TEST_F(Spc700Test, TEST_DIV_YA_X)
{
    this->RunInstructionTest(this->test_info_->name(), "9E");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_AND_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "24");
}

TEST_F(Spc700Test, TEST_AND_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "25");
}

TEST_F(Spc700Test, TEST_AND_IndirectX)
{
    this->RunInstructionTest(this->test_info_->name(), "26");
}

TEST_F(Spc700Test, TEST_AND_IndirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "27");
}

TEST_F(Spc700Test, TEST_AND_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "28");
}

TEST_F(Spc700Test, TEST_AND_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "34");
}

TEST_F(Spc700Test, TEST_AND_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "35");
}

TEST_F(Spc700Test, TEST_AND_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "36");
}

TEST_F(Spc700Test, TEST_AND_IndirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "37");
}

TEST_F(Spc700Test, TEST_AND_IndirectToIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "39");
}

TEST_F(Spc700Test, TEST_AND_DP_DP)
{
    this->RunInstructionTest(this->test_info_->name(), "29");
}

TEST_F(Spc700Test, TEST_AND_DP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "38");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_OR_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "04");
}

TEST_F(Spc700Test, TEST_OR_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "05");
}

TEST_F(Spc700Test, TEST_OR_IndirectX)
{
    this->RunInstructionTest(this->test_info_->name(), "06");
}

TEST_F(Spc700Test, TEST_OR_IndirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "07");
}

TEST_F(Spc700Test, TEST_OR_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "08");
}

TEST_F(Spc700Test, TEST_OR_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "14");
}

TEST_F(Spc700Test, TEST_OR_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "15");
}

TEST_F(Spc700Test, TEST_OR_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "16");
}

TEST_F(Spc700Test, TEST_OR_IndirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "17");
}

TEST_F(Spc700Test, TEST_OR_IndirectToIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "19");
}

TEST_F(Spc700Test, TEST_OR_DP_DP)
{
    this->RunInstructionTest(this->test_info_->name(), "09");
}

TEST_F(Spc700Test, TEST_OR_DP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "18");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_EOR_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "44");
}

TEST_F(Spc700Test, TEST_EOR_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "45");
}

TEST_F(Spc700Test, TEST_EOR_IndirectX)
{
    this->RunInstructionTest(this->test_info_->name(), "46");
}

TEST_F(Spc700Test, TEST_EOR_IndirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "47");
}

TEST_F(Spc700Test, TEST_EOR_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "48");
}

TEST_F(Spc700Test, TEST_EOR_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "54");
}

TEST_F(Spc700Test, TEST_EOR_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "55");
}

TEST_F(Spc700Test, TEST_EOR_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "56");
}

TEST_F(Spc700Test, TEST_EOR_IndirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "57");
}

TEST_F(Spc700Test, TEST_EOR_IndirectToIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "59");
}

TEST_F(Spc700Test, TEST_EOR_DP_DP)
{
    this->RunInstructionTest(this->test_info_->name(), "49");
}

TEST_F(Spc700Test, TEST_EOR_DP_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "58");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_SET1_DP_0)
{
    this->RunInstructionTest(this->test_info_->name(), "02");
}

TEST_F(Spc700Test, TEST_SET1_DP_1)
{
    this->RunInstructionTest(this->test_info_->name(), "22");
}

TEST_F(Spc700Test, TEST_SET1_DP_2)
{
    this->RunInstructionTest(this->test_info_->name(), "42");
}

TEST_F(Spc700Test, TEST_SET1_DP_3)
{
    this->RunInstructionTest(this->test_info_->name(), "62");
}

TEST_F(Spc700Test, TEST_SET1_DP_4)
{
    this->RunInstructionTest(this->test_info_->name(), "82");
}

TEST_F(Spc700Test, TEST_SET1_DP_5)
{
    this->RunInstructionTest(this->test_info_->name(), "A2");
}

TEST_F(Spc700Test, TEST_SET1_DP_6)
{
    this->RunInstructionTest(this->test_info_->name(), "C2");
}

TEST_F(Spc700Test, TEST_SET1_DP_7)
{
    this->RunInstructionTest(this->test_info_->name(), "E2");
}

TEST_F(Spc700Test, TEST_CLR1_DP_0)
{
    this->RunInstructionTest(this->test_info_->name(), "12");
}

TEST_F(Spc700Test, TEST_CLR1_DP_1)
{
    this->RunInstructionTest(this->test_info_->name(), "32");
}

TEST_F(Spc700Test, TEST_CLR1_DP_2)
{
    this->RunInstructionTest(this->test_info_->name(), "52");
}

TEST_F(Spc700Test, TEST_CLR1_DP_3)
{
    this->RunInstructionTest(this->test_info_->name(), "72");
}

TEST_F(Spc700Test, TEST_CLR1_DP_4)
{
    this->RunInstructionTest(this->test_info_->name(), "92");
}

TEST_F(Spc700Test, TEST_CLR1_DP_5)
{
    this->RunInstructionTest(this->test_info_->name(), "B2");
}

TEST_F(Spc700Test, TEST_CLR1_DP_6)
{
    this->RunInstructionTest(this->test_info_->name(), "D2");
}

TEST_F(Spc700Test, TEST_CLR1_DP_7)
{
    this->RunInstructionTest(this->test_info_->name(), "F2");
}

TEST_F(Spc700Test, TEST_TSET1)
{
    this->RunInstructionTest(this->test_info_->name(), "0E");
}

TEST_F(Spc700Test, TEST_TCLR1)
{
    this->RunInstructionTest(this->test_info_->name(), "4E");
}

TEST_F(Spc700Test, TEST_AND1)
{
    this->RunInstructionTest(this->test_info_->name(), "4A");
}

TEST_F(Spc700Test, TEST_AND1_not)
{
    this->RunInstructionTest(this->test_info_->name(), "6A");
}

TEST_F(Spc700Test, TEST_OR1)
{
    this->RunInstructionTest(this->test_info_->name(), "0A");
}

TEST_F(Spc700Test, TEST_OR1_not)
{
    this->RunInstructionTest(this->test_info_->name(), "2A");
}

TEST_F(Spc700Test, TEST_EOR1_not)
{
    this->RunInstructionTest(this->test_info_->name(), "8A");
}

TEST_F(Spc700Test, TEST_NOT1_not)
{
    this->RunInstructionTest(this->test_info_->name(), "EA");
}

TEST_F(Spc700Test, TEST_MOV1_C_Abs)
{
    this->RunInstructionTest(this->test_info_->name(), "AA");
}

TEST_F(Spc700Test, TEST_MOV1_Abs_C)
{
    this->RunInstructionTest(this->test_info_->name(), "CA");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_INC_A)
{
    this->RunInstructionTest(this->test_info_->name(), "BC");
}

TEST_F(Spc700Test, TEST_INC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "AB");
}

TEST_F(Spc700Test, TEST_INC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "BB");
}

TEST_F(Spc700Test, TEST_INC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "AC");
}

TEST_F(Spc700Test, TEST_INC_X)
{
    this->RunInstructionTest(this->test_info_->name(), "3D");
}

TEST_F(Spc700Test, TEST_INC_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "FC");
}

TEST_F(Spc700Test, TEST_INCW_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "3A");
}

TEST_F(Spc700Test, TEST_DEC_A)
{
    this->RunInstructionTest(this->test_info_->name(), "9C");
}

TEST_F(Spc700Test, TEST_DEC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "8B");
}

TEST_F(Spc700Test, TEST_DEC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "9B");
}

TEST_F(Spc700Test, TEST_DEC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "8C");
}

TEST_F(Spc700Test, TEST_DEC_X)
{
    this->RunInstructionTest(this->test_info_->name(), "1D");
}

TEST_F(Spc700Test, TEST_DEC_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "DC");
}

TEST_F(Spc700Test, TEST_DECW_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "1A");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_ASL_A)
{
    this->RunInstructionTest(this->test_info_->name(), "1C");
}

TEST_F(Spc700Test, TEST_ASL_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "0B");
}

TEST_F(Spc700Test, TEST_ASL_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "1B");
}

TEST_F(Spc700Test, TEST_ASL_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "0C");
}

TEST_F(Spc700Test, TEST_LSR_A)
{
    this->RunInstructionTest(this->test_info_->name(), "5C");
}

TEST_F(Spc700Test, TEST_LSR_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "4B");
}

TEST_F(Spc700Test, TEST_LSR_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "5B");
}

TEST_F(Spc700Test, TEST_LSR_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "4C");
}

TEST_F(Spc700Test, TEST_ROL_A)
{
    this->RunInstructionTest(this->test_info_->name(), "3C");
}

TEST_F(Spc700Test, TEST_ROL_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "2B");
}

TEST_F(Spc700Test, TEST_ROL_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "3B");
}

TEST_F(Spc700Test, TEST_ROL_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "2C");
}

TEST_F(Spc700Test, TEST_ROR_A)
{
    this->RunInstructionTest(this->test_info_->name(), "7C");
}

TEST_F(Spc700Test, TEST_ROR_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "6B");
}

TEST_F(Spc700Test, TEST_ROR_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "7B");
}

TEST_F(Spc700Test, TEST_ROR_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "6C");
}

TEST_F(Spc700Test, TEST_XCN)
{
    this->RunInstructionTest(this->test_info_->name(), "9F");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_PUSH_A)
{
    this->RunInstructionTest(this->test_info_->name(), "2D");
}

TEST_F(Spc700Test, TEST_PUSH_X)
{
    this->RunInstructionTest(this->test_info_->name(), "4D");
}

TEST_F(Spc700Test, TEST_PUSH_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "6D");
}

TEST_F(Spc700Test, TEST_PUSH_PSW)
{
    this->RunInstructionTest(this->test_info_->name(), "0D");
}

TEST_F(Spc700Test, TEST_POP_A)
{
    this->RunInstructionTest(this->test_info_->name(), "AE");
}

TEST_F(Spc700Test, TEST_POP_X)
{
    this->RunInstructionTest(this->test_info_->name(), "CE");
}

TEST_F(Spc700Test, TEST_POP_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "EE");
}

TEST_F(Spc700Test, TEST_POP_PSW)
{
    this->RunInstructionTest(this->test_info_->name(), "8E");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_BRA)
{
    this->RunInstructionTest(this->test_info_->name(), "2F");
}

TEST_F(Spc700Test, TEST_BPL)
{
    this->RunInstructionTest(this->test_info_->name(), "10");
}

TEST_F(Spc700Test, TEST_BMI)
{
    this->RunInstructionTest(this->test_info_->name(), "30");
}

TEST_F(Spc700Test, TEST_BVC)
{
    this->RunInstructionTest(this->test_info_->name(), "50");
}

TEST_F(Spc700Test, TEST_BVS)
{
    this->RunInstructionTest(this->test_info_->name(), "70");
}

TEST_F(Spc700Test, TEST_BCC)
{
    this->RunInstructionTest(this->test_info_->name(), "90");
}

TEST_F(Spc700Test, TEST_BCS)
{
    this->RunInstructionTest(this->test_info_->name(), "B0");
}

TEST_F(Spc700Test, TEST_BNE)
{
    this->RunInstructionTest(this->test_info_->name(), "D0");
}

TEST_F(Spc700Test, TEST_BEQ)
{
    this->RunInstructionTest(this->test_info_->name(), "F0");
}

TEST_F(Spc700Test, TEST_BBS_0)
{
    this->RunInstructionTest(this->test_info_->name(), "03");
}

TEST_F(Spc700Test, TEST_BBS_1)
{
    this->RunInstructionTest(this->test_info_->name(), "23");
}

TEST_F(Spc700Test, TEST_BBS_2)
{
    this->RunInstructionTest(this->test_info_->name(), "43");
}

TEST_F(Spc700Test, TEST_BBS_3)
{
    this->RunInstructionTest(this->test_info_->name(), "63");
}

TEST_F(Spc700Test, TEST_BBS_4)
{
    this->RunInstructionTest(this->test_info_->name(), "83");
}

TEST_F(Spc700Test, TEST_BBS_5)
{
    this->RunInstructionTest(this->test_info_->name(), "A3");
}

TEST_F(Spc700Test, TEST_BBS_6)
{
    this->RunInstructionTest(this->test_info_->name(), "C3");
}

TEST_F(Spc700Test, TEST_BBS_7)
{
    this->RunInstructionTest(this->test_info_->name(), "E3");
}

TEST_F(Spc700Test, TEST_BBC_0)
{
    this->RunInstructionTest(this->test_info_->name(), "13");
}

TEST_F(Spc700Test, TEST_BBC_1)
{
    this->RunInstructionTest(this->test_info_->name(), "33");
}

TEST_F(Spc700Test, TEST_BBC_2)
{
    this->RunInstructionTest(this->test_info_->name(), "53");
}

TEST_F(Spc700Test, TEST_BBC_3)
{
    this->RunInstructionTest(this->test_info_->name(), "73");
}

TEST_F(Spc700Test, TEST_BBC_4)
{
    this->RunInstructionTest(this->test_info_->name(), "93");
}

TEST_F(Spc700Test, TEST_BBC_5)
{
    this->RunInstructionTest(this->test_info_->name(), "B3");
}

TEST_F(Spc700Test, TEST_BBC_6)
{
    this->RunInstructionTest(this->test_info_->name(), "D3");
}

TEST_F(Spc700Test, TEST_BBC_7)
{
    this->RunInstructionTest(this->test_info_->name(), "F3");
}

TEST_F(Spc700Test, TEST_CBNE_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "2E");
}

TEST_F(Spc700Test, TEST_CBNE_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "DE");
}

TEST_F(Spc700Test, TEST_DBNZ_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "6E");
}

TEST_F(Spc700Test, TEST_DBNZ_Y)
{
    this->RunInstructionTest(this->test_info_->name(), "FE");
}

TEST_F(Spc700Test, TEST_JMP_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "5F");
}

TEST_F(Spc700Test, TEST_JMP_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "1F");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_CALL)
{
    this->RunInstructionTest(this->test_info_->name(), "3F");
}

TEST_F(Spc700Test, TEST_PCALL)
{
    this->RunInstructionTest(this->test_info_->name(), "4F");
}

TEST_F(Spc700Test, TEST_TCALL_0)
{
    this->RunInstructionTest(this->test_info_->name(), "01");
}

TEST_F(Spc700Test, TEST_TCALL_1)
{
    this->RunInstructionTest(this->test_info_->name(), "11");
}

TEST_F(Spc700Test, TEST_TCALL_2)
{
    this->RunInstructionTest(this->test_info_->name(), "21");
}

TEST_F(Spc700Test, TEST_TCALL_3)
{
    this->RunInstructionTest(this->test_info_->name(), "31");
}

TEST_F(Spc700Test, TEST_TCALL_4)
{
    this->RunInstructionTest(this->test_info_->name(), "41");
}

TEST_F(Spc700Test, TEST_TCALL_5)
{
    this->RunInstructionTest(this->test_info_->name(), "51");
}

TEST_F(Spc700Test, TEST_TCALL_6)
{
    this->RunInstructionTest(this->test_info_->name(), "61");
}

TEST_F(Spc700Test, TEST_TCALL_7)
{
    this->RunInstructionTest(this->test_info_->name(), "71");
}

TEST_F(Spc700Test, TEST_TCALL_8)
{
    this->RunInstructionTest(this->test_info_->name(), "81");
}

TEST_F(Spc700Test, TEST_TCALL_9)
{
    this->RunInstructionTest(this->test_info_->name(), "91");
}

TEST_F(Spc700Test, TEST_TCALL_A)
{
    this->RunInstructionTest(this->test_info_->name(), "A1");
}

TEST_F(Spc700Test, TEST_TCALL_B)
{
    this->RunInstructionTest(this->test_info_->name(), "B1");
}

TEST_F(Spc700Test, TEST_TCALL_C)
{
    this->RunInstructionTest(this->test_info_->name(), "C1");
}

TEST_F(Spc700Test, TEST_TCALL_D)
{
    this->RunInstructionTest(this->test_info_->name(), "D1");
}

TEST_F(Spc700Test, TEST_TCALL_E)
{
    this->RunInstructionTest(this->test_info_->name(), "E1");
}

TEST_F(Spc700Test, TEST_TCALL_F)
{
    this->RunInstructionTest(this->test_info_->name(), "F1");
}

TEST_F(Spc700Test, TEST_BRK)
{
    this->RunInstructionTest(this->test_info_->name(), "0F");
}

TEST_F(Spc700Test, TEST_RET)
{
    this->RunInstructionTest(this->test_info_->name(), "6F");
}

TEST_F(Spc700Test, TEST_RETI)
{
    this->RunInstructionTest(this->test_info_->name(), "7F");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_CLRC)
{
    this->RunInstructionTest(this->test_info_->name(), "60");
}

TEST_F(Spc700Test, TEST_SETC)
{
    this->RunInstructionTest(this->test_info_->name(), "80");
}

TEST_F(Spc700Test, TEST_NOTC)
{
    this->RunInstructionTest(this->test_info_->name(), "ED");
}

TEST_F(Spc700Test, TEST_CLRV)
{
    this->RunInstructionTest(this->test_info_->name(), "E0");
}

TEST_F(Spc700Test, TEST_CLRP)
{
    this->RunInstructionTest(this->test_info_->name(), "20");
}

TEST_F(Spc700Test, TEST_SETP)
{
    this->RunInstructionTest(this->test_info_->name(), "40");
}

TEST_F(Spc700Test, TEST_EI)
{
    this->RunInstructionTest(this->test_info_->name(), "A0");
}

TEST_F(Spc700Test, TEST_DI)
{
    this->RunInstructionTest(this->test_info_->name(), "C0");
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(Spc700Test, TEST_DAA)
{
    this->RunInstructionTest(this->test_info_->name(), "DF");
}

TEST_F(Spc700Test, TEST_DAS)
{
    this->RunInstructionTest(this->test_info_->name(), "BE");
}

TEST_F(Spc700Test, TEST_NOP)
{
    this->RunInstructionTest(this->test_info_->name(), "00");
}

TEST_F(Spc700Test, TEST_SLEEP)
{
    this->RunInstructionTest(this->test_info_->name(), "EF");
}

TEST_F(Spc700Test, TEST_STOP)
{
    this->RunInstructionTest(this->test_info_->name(), "FF");
}

///////////////////////////////////////////////////////////////////////////////

} // end namespace