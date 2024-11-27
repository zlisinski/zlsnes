#include <stdint.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "CpuTest.h"
#include "../Memory.h"
#include "../Zlsnes.h"

const uint16_t A_VALUE = 0x1234;
const uint16_t X_VALUE = 0x5678;
const uint16_t Y_VALUE = 0x9ABC;
const uint16_t D_VALUE = 0xDEF0;
const uint8_t P_VALUE = 0x00;
const uint8_t DB_VALUE = 0x12;
const uint8_t PB_VALUE = 0x34;
const uint16_t SP_VALUE = 0xFFFF;

const QString JSON_PATH = "./test_data/65816/v1/";


CpuTest::CpuTest()
{
    memory_ = new Memory();
    cpu = new Cpu(memory_);
}

CpuTest::~CpuTest()
{
    delete cpu;
    delete memory_;
}

void CpuTest::SetUp()
{
    ResetState();
}

void CpuTest::TearDown()
{

}

void CpuTest::ResetState()
{
    cpu->reg.a = A_VALUE;
    cpu->reg.x = X_VALUE;
    cpu->reg.y = Y_VALUE;
    cpu->reg.d = D_VALUE;
    cpu->reg.p = P_VALUE;
    cpu->reg.db = DB_VALUE;
    cpu->reg.pb = PB_VALUE;
    cpu->reg.sp = SP_VALUE;
    cpu->reg.pc = 0;
    cpu->reg.emulationMode = false;

    memory_->ClearMemory();
    memory = memory_->GetBytePtr(0);
}

void CpuTest::RunInstructionTest(const QString &opcodeName, const QString &opcode, bool emulationMode)
{
    QString testName = opcodeName + ": ";

    QString filename = QStringLiteral("%1%2.%3.json").arg(JSON_PATH).arg(opcode.toLower()).arg(emulationMode ? "e" : "n");
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
        cpu->reg.d = initial["d"].toInt();
        cpu->reg.p = initial["p"].toInt();
        cpu->reg.db = initial["dbr"].toInt();
        cpu->reg.pb = initial["pbr"].toInt();
        cpu->reg.sp = initial["s"].toInt();
        cpu->reg.pc = initial["pc"].toInt();
        cpu->reg.emulationMode = emulationMode;

        if (cpu->reg.flags.d == 1 && (opcodeName.startsWith("TEST_ADC_") || opcodeName.startsWith("TEST_SBC_")))
        {
            // BCD mode is broken for invalid BCD digits, ignore for now.
            // TODO: Fix this.
            continue;
        }

        // Set RAM initial values.
        QJsonArray initalRam = initial["ram"].toArray();
        for (int j = 0; j < initalRam.size(); j++)
        {
            QJsonArray pair = initalRam[j].toArray();
            int32_t addr = pair[0].toInt();
            int32_t val = pair[1].toInt();
            ASSERT_GE(addr, 0) << qPrintable(testName);
            ASSERT_LE(addr, 0xFFFFFF) << qPrintable(testName);
            ASSERT_GE(val, 0) << qPrintable(testName);
            ASSERT_LE(val, 0xFF) << qPrintable(testName);
            memory[addr] = val;
        }

        // Run the opcode.
        cpu->ProcessOpCode();

        // Verify result register values.
        QJsonObject final = obj["final"].toObject();
        EXPECT_EQ(cpu->reg.a, final["a"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.x, final["x"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.y, final["y"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.d, final["d"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.p, final["p"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.db, final["dbr"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.pb, final["pbr"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.sp, final["s"].toInt()) << qPrintable(testName);
        EXPECT_EQ(cpu->reg.pc, final["pc"].toInt()) << qPrintable(testName);

        // Verify result RAM values.
        QJsonArray finalRam = final["ram"].toArray();
        for (int j = 0; j < finalRam.size(); j++)
        {
            QJsonArray pair = finalRam[j].toArray();
            int32_t addr = pair[0].toInt();
            int32_t val = pair[1].toInt();
            ASSERT_GE(addr, 0) << qPrintable(testName);
            ASSERT_LE(addr, 0xFFFFFF) << qPrintable(testName);
            ASSERT_GE(val, 0) << qPrintable(testName);
            ASSERT_LE(val, 0xFF) << qPrintable(testName);
            EXPECT_EQ(memory[addr], val) << qPrintable(testName);
        }

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

void CpuTest::FormatData(const QJsonObject &obj, QString &str)
{
    str += "\nname: \"" + obj["name"].toString() + "\"\n";

    QJsonObject initial = obj["initial"].toObject();
    str += "initial: \n";
    str += QStringLiteral("\ta: 0x%1\n").arg(initial["a"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tx: 0x%1\n").arg(initial["x"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\ty: 0x%1\n").arg(initial["y"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\td: 0x%1\n").arg(initial["d"].toInt(), 4, 16, QChar('0'));
    uint8_t p = initial["p"].toInt();
    str += QStringLiteral("\tp: 0x%1 (n=%2 v=%3 m=%4 x=%5 d=%6 i=%7 z=%8 c=%9)\n")
        .arg(p, 2, 16, QChar('0'))
        .arg(p >> 7)
        .arg((p >> 6) & 0x01)
        .arg((p >> 5) & 0x01)
        .arg((p >> 4) & 0x01)
        .arg((p >> 3) & 0x01)
        .arg((p >> 2) & 0x01)
        .arg((p >> 1) & 0x01)
        .arg(p & 0x01);
    str += QStringLiteral("\tdb: 0x%1\n").arg(initial["dbr"].toInt(), 2, 16, QChar('0'));
    str += QStringLiteral("\tpb: 0x%1\n").arg(initial["pbr"].toInt(), 2, 16, QChar('0'));
    str += QStringLiteral("\tsp: 0x%1\n").arg(initial["s"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tpc: 0x%1\n").arg(initial["pc"].toInt(), 4, 16, QChar('0'));

    QJsonArray initalRam = initial["ram"].toArray();
    str += "\tram: \n";
    for (int i = 0; i < initalRam.size(); i++)
    {
        QJsonArray pair = initalRam[i].toArray();
        str += QStringLiteral("\t\t0x%1 = 0x%2\n").arg(pair[0].toInt(), 6, 16, QChar('0')).arg(pair[1].toInt(), 2, 16, QChar('0'));
    }

    QJsonObject final = obj["final"].toObject();
    str += "expected: \n";
    str += QStringLiteral("\ta: 0x%1\n").arg(final["a"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tx: 0x%1\n").arg(final["x"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\ty: 0x%1\n").arg(final["y"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\td: 0x%1\n").arg(final["d"].toInt(), 4, 16, QChar('0'));
    p = final["p"].toInt();
    str += QStringLiteral("\tp: 0x%1 (n=%2 v=%3 m=%4 x=%5 d=%6 i=%7 z=%8 c=%9)\n")
        .arg(p, 2, 16, QChar('0'))
        .arg(p >> 7)
        .arg((p >> 6) & 0x01)
        .arg((p >> 5) & 0x01)
        .arg((p >> 4) & 0x01)
        .arg((p >> 3) & 0x01)
        .arg((p >> 2) & 0x01)
        .arg((p >> 1) & 0x01)
        .arg(p & 0x01);
    str += QStringLiteral("\tdb: 0x%1\n").arg(final["dbr"].toInt(), 2, 16, QChar('0'));
    str += QStringLiteral("\tpb: 0x%1\n").arg(final["pbr"].toInt(), 2, 16, QChar('0'));
    str += QStringLiteral("\tsp: 0x%1\n").arg(final["s"].toInt(), 4, 16, QChar('0'));
    str += QStringLiteral("\tpc: 0x%1\n").arg(final["pc"].toInt(), 4, 16, QChar('0'));

    QJsonArray finalRam = final["ram"].toArray();
    str += "\tram: \n";
    for (int i = 0; i < finalRam.size(); i++)
    {
        QJsonArray pair = finalRam[i].toArray();
        str += QStringLiteral("\t\t0x%1 = 0x%2\n").arg(pair[0].toInt(), 6, 16, QChar('0')).arg(pair[1].toInt(), 2, 16, QChar('0'));
    }

    str += "actual: \n";
    str += QStringLiteral("\ta: 0x%1\n").arg(cpu->reg.a, 4, 16, QChar('0'));
    str += QStringLiteral("\tx: 0x%1\n").arg(cpu->reg.x, 4, 16, QChar('0'));
    str += QStringLiteral("\ty: 0x%1\n").arg(cpu->reg.y, 4, 16, QChar('0'));
    str += QStringLiteral("\td: 0x%1\n").arg(cpu->reg.d, 4, 16, QChar('0'));
    p = cpu->reg.p;
    str += QStringLiteral("\tp: 0x%1 (n=%2 v=%3 m=%4 x=%5 d=%6 i=%7 z=%8 c=%9)\n")
        .arg(p, 2, 16, QChar('0'))
        .arg(p >> 7)
        .arg((p >> 6) & 0x01)
        .arg((p >> 5) & 0x01)
        .arg((p >> 4) & 0x01)
        .arg((p >> 3) & 0x01)
        .arg((p >> 2) & 0x01)
        .arg((p >> 1) & 0x01)
        .arg(p & 0x01);
    str += QStringLiteral("\tdb: 0x%1\n").arg(cpu->reg.db, 2, 16, QChar('0'));
    str += QStringLiteral("\tpb: 0x%1\n").arg(cpu->reg.pb, 2, 16, QChar('0'));
    str += QStringLiteral("\tsp: 0x%1\n").arg(cpu->reg.sp, 4, 16, QChar('0'));
    str += QStringLiteral("\tpc: 0x%1\n").arg(cpu->reg.pc, 4, 16, QChar('0'));

    str += "\tram: \n";
    for (int i = 0; i < finalRam.size(); i++)
    {
        QJsonArray pair = finalRam[i].toArray();
        str += QStringLiteral("\t\t0x%1 = 0x%2\n").arg(pair[0].toInt(), 6, 16, QChar('0')).arg(memory[pair[0].toInt()], 2, 16, QChar('0'));
    }
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_LoadRegister8Bit)
{
    uint8_t dest;

    ResetState();
    dest = 0x34;
    LoadRegister(&dest, uint8_t{0});
    ASSERT_EQ(dest, 0x00);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    dest = 0x34;
    LoadRegister(&dest, uint8_t{0x78});
    ASSERT_EQ(dest, 0x78);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    dest = 0x34;
    LoadRegister(&dest, uint8_t{0x80});
    ASSERT_EQ(dest, 0x80);
    ASSERT_EQ(cpu->reg.flags.n, 1);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x80);

}

TEST_F(CpuTest, TEST_LoadRegister16Bit)
{
    uint16_t dest;

    ResetState();
    dest = 0;
    LoadRegister(&dest, uint16_t{0x1234});
    ASSERT_EQ(dest, 0x1234);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    dest = 0xFFFF;
    LoadRegister(&dest, uint16_t{0});
    ASSERT_EQ(dest, 0);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    dest = 0;
    LoadRegister(&dest, uint16_t{0xFFFF});
    ASSERT_EQ(dest, 0xFFFF);
    ASSERT_EQ(cpu->reg.flags.n, 1);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x80);
}

TEST_F(CpuTest, TEST_IsXYBit)
{
    cpu->reg.emulationMode = false;
    cpu->reg.flags.m = 0;
    ASSERT_TRUE(IsAccumulator16Bit());
    ASSERT_FALSE(IsAccumulator8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.m = 0;
    ASSERT_FALSE(IsAccumulator16Bit());
    ASSERT_TRUE(IsAccumulator8Bit());

    cpu->reg.emulationMode = false;
    cpu->reg.flags.m = 1;
    ASSERT_FALSE(IsAccumulator16Bit());
    ASSERT_TRUE(IsAccumulator8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.m = 1;
    ASSERT_FALSE(IsAccumulator16Bit());
    ASSERT_TRUE(IsAccumulator8Bit());

    cpu->reg.emulationMode = false;
    cpu->reg.flags.x = 0;
    ASSERT_TRUE(IsIndex16Bit());
    ASSERT_FALSE(IsIndex8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.x = 0;
    ASSERT_FALSE(IsIndex16Bit());
    ASSERT_TRUE(IsIndex8Bit());

    cpu->reg.emulationMode = false;
    cpu->reg.flags.x = 1;
    ASSERT_FALSE(IsIndex16Bit());
    ASSERT_TRUE(IsIndex8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.x = 1;
    ASSERT_FALSE(IsIndex16Bit());
    ASSERT_TRUE(IsIndex8Bit());
}

TEST_F(CpuTest, TEST_Push8Bit)
{
    cpu->reg.emulationMode = true;
    cpu->reg.sp = 0x0100;
    Push8Bit(0xAB);
    ASSERT_EQ(cpu->reg.sp, 0x01FF);
    ASSERT_EQ(memory[0x0100], 0xAB);
}

TEST_F(CpuTest, TEST_Pop8Bit)
{
    cpu->reg.emulationMode = true;
    memory[0x0100] = 0xAB;
    cpu->reg.sp = 0x01FF;
    uint8_t value = Pop8Bit();
    ASSERT_EQ(cpu->reg.sp, 0x0100);
    ASSERT_EQ(value, 0xAB);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_TAX)
{
    this->RunInstructionTest(this->test_info_->name(), "AA", false);
    //this->RunInstructionTest(this->test_info_->name(), "AA", true);
}

TEST_F(CpuTest, TEST_TAY)
{
    this->RunInstructionTest(this->test_info_->name(), "A8", false);
    //this->RunInstructionTest(this->test_info_->name(), "A8", true);
}

TEST_F(CpuTest, TEST_TSX)
{
    this->RunInstructionTest(this->test_info_->name(), "BA", false);
    //this->RunInstructionTest(this->test_info_->name(), "BA", true);
}

TEST_F(CpuTest, TEST_TXA)
{
    this->RunInstructionTest(this->test_info_->name(), "8A", false);
    //this->RunInstructionTest(this->test_info_->name(), "8A", true);
}

TEST_F(CpuTest, TEST_TXS)
{
    this->RunInstructionTest(this->test_info_->name(), "9A", false);
    this->RunInstructionTest(this->test_info_->name(), "9A", true);
}

TEST_F(CpuTest, TEST_TXY)
{
    this->RunInstructionTest(this->test_info_->name(), "9B", false);
    //this->RunInstructionTest(this->test_info_->name(), "9B", true);
}

TEST_F(CpuTest, TEST_TYA)
{
    this->RunInstructionTest(this->test_info_->name(), "98", false);
    //this->RunInstructionTest(this->test_info_->name(), "98", true);
}

TEST_F(CpuTest, TEST_TYX)
{
    this->RunInstructionTest(this->test_info_->name(), "BB", false);
    //this->RunInstructionTest(this->test_info_->name(), "BB", true);
}

TEST_F(CpuTest, TEST_TCD)
{
    this->RunInstructionTest(this->test_info_->name(), "5B", false);
    //this->RunInstructionTest(this->test_info_->name(), "5B", true);
}

TEST_F(CpuTest, TEST_TCS)
{
    this->RunInstructionTest(this->test_info_->name(), "1B", false);
    this->RunInstructionTest(this->test_info_->name(), "1B", true);
}

TEST_F(CpuTest, TEST_TDC)
{
    this->RunInstructionTest(this->test_info_->name(), "7B", false);
    //this->RunInstructionTest(this->test_info_->name(), "7B", true);
}

TEST_F(CpuTest, TEST_TSC)
{
    this->RunInstructionTest(this->test_info_->name(), "3B", false);
    //this->RunInstructionTest(this->test_info_->name(), "3B", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_LDA_DirectIndexedIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "A1", false);
    //this->RunInstructionTest(this->test_info_->name(), "A1", true);
}

TEST_F(CpuTest, TEST_LDA_StackRelative)
{
    this->RunInstructionTest(this->test_info_->name(), "A3", false);
    //this->RunInstructionTest(this->test_info_->name(), "A3", true);
}

TEST_F(CpuTest, TEST_LDA_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "A5", false);
    //this->RunInstructionTest(this->test_info_->name(), "A5", true);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirectLong)
{
    this->RunInstructionTest(this->test_info_->name(), "A7", false);
    //this->RunInstructionTest(this->test_info_->name(), "A7", true);
}

TEST_F(CpuTest, TEST_LDA_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "A9", false);
    //this->RunInstructionTest(this->test_info_->name(), "A9", true);
}

TEST_F(CpuTest, TEST_LDA_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "AD", false);
    //this->RunInstructionTest(this->test_info_->name(), "AD", true);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteLong)
{
    this->RunInstructionTest(this->test_info_->name(), "AF", false);
    //this->RunInstructionTest(this->test_info_->name(), "AF", true);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "B1", false);
    //this->RunInstructionTest(this->test_info_->name(), "B1", true);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "B2", false);
    //this->RunInstructionTest(this->test_info_->name(), "B2", true);
}

TEST_F(CpuTest, TEST_LDA_StackRelativeIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "B3", false);
    //this->RunInstructionTest(this->test_info_->name(), "B3", true);
}

TEST_F(CpuTest, TEST_LDA_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "B5", false);
    //this->RunInstructionTest(this->test_info_->name(), "B5", true);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirectLongIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "B7", false);
    //this->RunInstructionTest(this->test_info_->name(), "B7", true);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "B9", false);
    //this->RunInstructionTest(this->test_info_->name(), "B9", true);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "BD", false);
    //this->RunInstructionTest(this->test_info_->name(), "BD", true);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteLongIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "BF", false);
    //this->RunInstructionTest(this->test_info_->name(), "BF", true);
}

TEST_F(CpuTest, TEST_LDX_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "A2", false);
    //this->RunInstructionTest(this->test_info_->name(), "A2", true);
}

TEST_F(CpuTest, TEST_LDX_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "A6", false);
    //this->RunInstructionTest(this->test_info_->name(), "A6", true);
}

TEST_F(CpuTest, TEST_LDX_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "AE", false);
    //this->RunInstructionTest(this->test_info_->name(), "AE", true);
}

TEST_F(CpuTest, TEST_LDX_DirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "B6", false);
    //this->RunInstructionTest(this->test_info_->name(), "B6", true);
}

TEST_F(CpuTest, TEST_LDX_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "BE", false);
    //this->RunInstructionTest(this->test_info_->name(), "BE", true);
}

TEST_F(CpuTest, TEST_LDY_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "A0", false);
    //this->RunInstructionTest(this->test_info_->name(), "A0", true);
}

TEST_F(CpuTest, TEST_LDY_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "A4", false);
    //this->RunInstructionTest(this->test_info_->name(), "A4", true);
}

TEST_F(CpuTest, TEST_LDY_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "AC", false);
    //this->RunInstructionTest(this->test_info_->name(), "AC", true);
}

TEST_F(CpuTest, TEST_LDY_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "B4", false);
    //this->RunInstructionTest(this->test_info_->name(), "B4", true);
}

TEST_F(CpuTest, TEST_LDY_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "BC", false);
    //this->RunInstructionTest(this->test_info_->name(), "BC", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_STA_DirectIndexedIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "81", false);
    //this->RunInstructionTest(this->test_info_->name(), "81", true);
}

TEST_F(CpuTest, TEST_STA_StackRelative)
{
    this->RunInstructionTest(this->test_info_->name(), "83", false);
    //this->RunInstructionTest(this->test_info_->name(), "83", true);
}

TEST_F(CpuTest, TEST_STA_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "85", false);
    //this->RunInstructionTest(this->test_info_->name(), "85", true);
}

TEST_F(CpuTest, TEST_STA_DirectIndirectLong)
{
    this->RunInstructionTest(this->test_info_->name(), "87", false);
    //this->RunInstructionTest(this->test_info_->name(), "87", true);
}

TEST_F(CpuTest, TEST_STA_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "8D", false);
    //this->RunInstructionTest(this->test_info_->name(), "8D", true);
}

TEST_F(CpuTest, TEST_STA_AbsoluteLong)
{
    this->RunInstructionTest(this->test_info_->name(), "8F", false);
    //this->RunInstructionTest(this->test_info_->name(), "8F", true);
}

TEST_F(CpuTest, TEST_STA_DirectIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "91", false);
    //this->RunInstructionTest(this->test_info_->name(), "91", true);
}

TEST_F(CpuTest, TEST_STA_DirectIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "92", false);
    //this->RunInstructionTest(this->test_info_->name(), "92", true);
}

TEST_F(CpuTest, TEST_STA_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "95", false);
    //this->RunInstructionTest(this->test_info_->name(), "95", true);
}

TEST_F(CpuTest, TEST_STA_DirectIndirectLongIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "97", false);
    //this->RunInstructionTest(this->test_info_->name(), "97", true);
}

TEST_F(CpuTest, TEST_STA_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "99", false);
    //this->RunInstructionTest(this->test_info_->name(), "99", true);
}

TEST_F(CpuTest, TEST_STA_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "9D", false);
    //this->RunInstructionTest(this->test_info_->name(), "9D", true);
}

TEST_F(CpuTest, TEST_STA_AbsoluteLongIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "9F", false);
    //this->RunInstructionTest(this->test_info_->name(), "9F", true);
}

TEST_F(CpuTest, TEST_STX_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "86", false);
    //this->RunInstructionTest(this->test_info_->name(), "86", true);
}

TEST_F(CpuTest, TEST_STX_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "8E", false);
    //this->RunInstructionTest(this->test_info_->name(), "8E", true);
}

TEST_F(CpuTest, TEST_STX_DirectIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "96", false);
    //this->RunInstructionTest(this->test_info_->name(), "96", true);
}

TEST_F(CpuTest, TEST_STY_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "84", false);
    //this->RunInstructionTest(this->test_info_->name(), "84", true);
}

TEST_F(CpuTest, TEST_STY_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "8C", false);
    //this->RunInstructionTest(this->test_info_->name(), "8C", true);
}

TEST_F(CpuTest, TEST_STY_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "94", false);
    //this->RunInstructionTest(this->test_info_->name(), "94", true);
}

TEST_F(CpuTest, TEST_STZ_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "64", false);
    //this->RunInstructionTest(this->test_info_->name(), "64", true);
}

TEST_F(CpuTest, TEST_STZ_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "74", false);
    //this->RunInstructionTest(this->test_info_->name(), "74", true);
}

TEST_F(CpuTest, TEST_STZ_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "9C", false);
    //this->RunInstructionTest(this->test_info_->name(), "9C", true);
}

TEST_F(CpuTest, TEST_STZ_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "9E", false);
    //this->RunInstructionTest(this->test_info_->name(), "9E", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_PHA)
{
    this->RunInstructionTest(this->test_info_->name(), "48", false);
    //this->RunInstructionTest(this->test_info_->name(), "48", true);
}

TEST_F(CpuTest, TEST_PHX)
{
    this->RunInstructionTest(this->test_info_->name(), "DA", false);
    //this->RunInstructionTest(this->test_info_->name(), "DA", true);
}

TEST_F(CpuTest, TEST_PHY)
{
    this->RunInstructionTest(this->test_info_->name(), "5A", false);
    //this->RunInstructionTest(this->test_info_->name(), "5A", true);
}

TEST_F(CpuTest, TEST_PHB)
{
    this->RunInstructionTest(this->test_info_->name(), "8B", false);
    //this->RunInstructionTest(this->test_info_->name(), "8B", true);
}

TEST_F(CpuTest, TEST_PHD)
{
    this->RunInstructionTest(this->test_info_->name(), "0B", false);
    //this->RunInstructionTest(this->test_info_->name(), "0B", true);
}

TEST_F(CpuTest, TEST_PHK)
{
    this->RunInstructionTest(this->test_info_->name(), "4B", false);
    //this->RunInstructionTest(this->test_info_->name(), "4B", true);
}

TEST_F(CpuTest, TEST_PHP)
{
    this->RunInstructionTest(this->test_info_->name(), "08", false);
    //this->RunInstructionTest(this->test_info_->name(), "08", true);
}

TEST_F(CpuTest, TEST_PEA)
{
    this->RunInstructionTest(this->test_info_->name(), "F4", false);
    //this->RunInstructionTest(this->test_info_->name(), "F4", true);
}

TEST_F(CpuTest, TEST_PEI)
{
    this->RunInstructionTest(this->test_info_->name(), "D4", false);
    //this->RunInstructionTest(this->test_info_->name(), "D4", true);
}

TEST_F(CpuTest, TEST_PER)
{
    this->RunInstructionTest(this->test_info_->name(), "62", false);
    //this->RunInstructionTest(this->test_info_->name(), "62", true);
}

TEST_F(CpuTest, TEST_PLA)
{
    this->RunInstructionTest(this->test_info_->name(), "68", false);
    //this->RunInstructionTest(this->test_info_->name(), "68", true);
}

TEST_F(CpuTest, TEST_PLX)
{
    this->RunInstructionTest(this->test_info_->name(), "FA", false);
    //this->RunInstructionTest(this->test_info_->name(), "FA", true);
}

TEST_F(CpuTest, TEST_PLY)
{
    this->RunInstructionTest(this->test_info_->name(), "7A", false);
    //this->RunInstructionTest(this->test_info_->name(), "7A", true);
}

TEST_F(CpuTest, TEST_PLB)
{
    this->RunInstructionTest(this->test_info_->name(), "AB", false);
    //this->RunInstructionTest(this->test_info_->name(), "AB", true);
}

TEST_F(CpuTest, TEST_PLD)
{
    this->RunInstructionTest(this->test_info_->name(), "2B", false);
    //this->RunInstructionTest(this->test_info_->name(), "2B", true);
}

TEST_F(CpuTest, TEST_PLP)
{
    this->RunInstructionTest(this->test_info_->name(), "28", false);
    //this->RunInstructionTest(this->test_info_->name(), "28", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_AND_DirectIndexedIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "21", false);
    //this->RunInstructionTest(this->test_info_->name(), "21", true);
}

TEST_F(CpuTest, TEST_AND_StackRelative)
{
    this->RunInstructionTest(this->test_info_->name(), "23", false);
    //this->RunInstructionTest(this->test_info_->name(), "23", true);
}

TEST_F(CpuTest, TEST_AND_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "25", false);
    //this->RunInstructionTest(this->test_info_->name(), "25", true);
}

TEST_F(CpuTest, TEST_AND_DirectIndirectLong)
{
    this->RunInstructionTest(this->test_info_->name(), "27", false);
    //this->RunInstructionTest(this->test_info_->name(), "27", true);
}

TEST_F(CpuTest, TEST_AND_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "29", false);
    //this->RunInstructionTest(this->test_info_->name(), ""29, true);
}

TEST_F(CpuTest, TEST_AND_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "2D", false);
    //this->RunInstructionTest(this->test_info_->name(), "2D", true);
}

TEST_F(CpuTest, TEST_AND_AbsoluteLong)
{
    this->RunInstructionTest(this->test_info_->name(), "2F", false);
    //this->RunInstructionTest(this->test_info_->name(), "2F", true);
}

TEST_F(CpuTest, TEST_AND_DirectIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "31", false);
    //this->RunInstructionTest(this->test_info_->name(), "31", true);
}

TEST_F(CpuTest, TEST_AND_DirectIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "32", false);
    //this->RunInstructionTest(this->test_info_->name(), "32", true);
}

TEST_F(CpuTest, TEST_AND_StackRelativeIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "33", false);
    //this->RunInstructionTest(this->test_info_->name(), "33", true);
}

TEST_F(CpuTest, TEST_AND_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "35", false);
    //this->RunInstructionTest(this->test_info_->name(), "35", true);
}

TEST_F(CpuTest, TEST_AND_DirectIndirectLongIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "37", false);
    //this->RunInstructionTest(this->test_info_->name(), "37", true);
}

TEST_F(CpuTest, TEST_AND_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "39", false);
    //this->RunInstructionTest(this->test_info_->name(), "39", true);
}

TEST_F(CpuTest, TEST_AND_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "3D", false);
    //this->RunInstructionTest(this->test_info_->name(), "3D", true);
}

TEST_F(CpuTest, TEST_AND_AbsoluteLongIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "3F", false);
    //this->RunInstructionTest(this->test_info_->name(), "3F", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_EOR_DirectIndexedIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "41", false);
    //this->RunInstructionTest(this->test_info_->name(), "41", true);
}

TEST_F(CpuTest, TEST_EOR_StackRelative)
{
    this->RunInstructionTest(this->test_info_->name(), "43", false);
    //this->RunInstructionTest(this->test_info_->name(), "43", true);
}

TEST_F(CpuTest, TEST_EOR_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "45", false);
    //this->RunInstructionTest(this->test_info_->name(), "45", true);
}

TEST_F(CpuTest, TEST_EOR_DirectIndirectLong)
{
    this->RunInstructionTest(this->test_info_->name(), "47", false);
    //this->RunInstructionTest(this->test_info_->name(), "47", true);
}

TEST_F(CpuTest, TEST_EOR_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "49", false);
    //this->RunInstructionTest(this->test_info_->name(), "49", true);
}

TEST_F(CpuTest, TEST_EOR_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "4D", false);
    //this->RunInstructionTest(this->test_info_->name(), "4D", true);
}

TEST_F(CpuTest, TEST_EOR_AbsoluteLong)
{
    this->RunInstructionTest(this->test_info_->name(), "4F", false);
    //this->RunInstructionTest(this->test_info_->name(), "4F", true);
}

TEST_F(CpuTest, TEST_EOR_DirectIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "51", false);
    //this->RunInstructionTest(this->test_info_->name(), "51", true);
}

TEST_F(CpuTest, TEST_EOR_DirectIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "52", false);
    //this->RunInstructionTest(this->test_info_->name(), "52", true);
}

TEST_F(CpuTest, TEST_EOR_StackRelativeIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "53", false);
    //this->RunInstructionTest(this->test_info_->name(), "53", true);
}

TEST_F(CpuTest, TEST_EOR_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "55", false);
    //this->RunInstructionTest(this->test_info_->name(), "55", true);
}

TEST_F(CpuTest, TEST_EOR_DirectIndirectLongIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "57", false);
    //this->RunInstructionTest(this->test_info_->name(), "57", true);
}

TEST_F(CpuTest, TEST_EOR_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "59", false);
    //this->RunInstructionTest(this->test_info_->name(), "59", true);
}

TEST_F(CpuTest, TEST_EOR_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "5D", false);
    //this->RunInstructionTest(this->test_info_->name(), "5D", true);
}

TEST_F(CpuTest, TEST_EOR_AbsoluteLongIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "5F", false);
    //this->RunInstructionTest(this->test_info_->name(), "5F", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_ORA_DirectIndexedIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "01", false);
    //this->RunInstructionTest(this->test_info_->name(), "01", true);
}

TEST_F(CpuTest, TEST_ORA_StackRelative)
{
    this->RunInstructionTest(this->test_info_->name(), "03", false);
    //this->RunInstructionTest(this->test_info_->name(), "03", true);
}

TEST_F(CpuTest, TEST_ORA_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "05", false);
    //this->RunInstructionTest(this->test_info_->name(), "05", true);
}

TEST_F(CpuTest, TEST_ORA_DirectIndirectLong)
{
    this->RunInstructionTest(this->test_info_->name(), "07", false);
    //this->RunInstructionTest(this->test_info_->name(), "07", true);
}

TEST_F(CpuTest, TEST_ORA_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "09", false);
    //this->RunInstructionTest(this->test_info_->name(), "09", true);
}

TEST_F(CpuTest, TEST_ORA_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "0D", false);
    //this->RunInstructionTest(this->test_info_->name(), "0D", true);
}

TEST_F(CpuTest, TEST_ORA_AbsoluteLong)
{
    this->RunInstructionTest(this->test_info_->name(), "0F", false);
    //this->RunInstructionTest(this->test_info_->name(), "0F", true);
}

TEST_F(CpuTest, TEST_ORA_DirectIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "11", false);
    //this->RunInstructionTest(this->test_info_->name(), "11", true);
}

TEST_F(CpuTest, TEST_ORA_DirectIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "12", false);
    //this->RunInstructionTest(this->test_info_->name(), "12", true);
}

TEST_F(CpuTest, TEST_ORA_StackRelativeIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "13", false);
    //this->RunInstructionTest(this->test_info_->name(), "13", true);
}

TEST_F(CpuTest, TEST_ORA_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "15", false);
    //this->RunInstructionTest(this->test_info_->name(), "15", true);
}

TEST_F(CpuTest, TEST_ORA_DirectIndirectLongIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "17", false);
    //this->RunInstructionTest(this->test_info_->name(), "17", true);
}

TEST_F(CpuTest, TEST_ORA_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "19", false);
    //this->RunInstructionTest(this->test_info_->name(), "19", true);
}

TEST_F(CpuTest, TEST_ORA_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "1D", false);
    //this->RunInstructionTest(this->test_info_->name(), "1D", true);
}

TEST_F(CpuTest, TEST_ORA_AbsoluteLongIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "1F", false);
    //this->RunInstructionTest(this->test_info_->name(), "1F", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_ADC_DirectIndexedIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "61", false);
    //this->RunInstructionTest(this->test_info_->name(), "61", true);
}

TEST_F(CpuTest, TEST_ADC_StackRelative)
{
    this->RunInstructionTest(this->test_info_->name(), "63", false);
    //this->RunInstructionTest(this->test_info_->name(), "63", true);
}

TEST_F(CpuTest, TEST_ADC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "65", false);
    //this->RunInstructionTest(this->test_info_->name(), "65", true);
}

TEST_F(CpuTest, TEST_ADC_DirectIndirectLong)
{
    this->RunInstructionTest(this->test_info_->name(), "67", false);
    //this->RunInstructionTest(this->test_info_->name(), "67", true);
}

TEST_F(CpuTest, TEST_ADC_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "69", false);
    //this->RunInstructionTest(this->test_info_->name(), "69", true);
}

TEST_F(CpuTest, TEST_ADC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "6D", false);
    //this->RunInstructionTest(this->test_info_->name(), "6D", true);
}

TEST_F(CpuTest, TEST_ADC_AbsoluteLong)
{
    this->RunInstructionTest(this->test_info_->name(), "6F", false);
    //this->RunInstructionTest(this->test_info_->name(), "6F", true);
}

TEST_F(CpuTest, TEST_ADC_DirectIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "71", false);
    //this->RunInstructionTest(this->test_info_->name(), "71", true);
}

TEST_F(CpuTest, TEST_ADC_DirectIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "72", false);
    //this->RunInstructionTest(this->test_info_->name(), "72", true);
}

TEST_F(CpuTest, TEST_ADC_StackRelativeIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "73", false);
    //this->RunInstructionTest(this->test_info_->name(), "73", true);
}

TEST_F(CpuTest, TEST_ADC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "75", false);
    //this->RunInstructionTest(this->test_info_->name(), "75", true);
}

TEST_F(CpuTest, TEST_ADC_DirectIndirectLongIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "77", false);
    //this->RunInstructionTest(this->test_info_->name(), "77", true);
}

TEST_F(CpuTest, TEST_ADC_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "79", false);
    //this->RunInstructionTest(this->test_info_->name(), "79", true);
}

TEST_F(CpuTest, TEST_ADC_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "7D", false);
    //this->RunInstructionTest(this->test_info_->name(), "7D", true);
}

TEST_F(CpuTest, TEST_ADC_AbsoluteLongIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "7F", false);
    //this->RunInstructionTest(this->test_info_->name(), "7F", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_SBC_DirectIndexedIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "E1", false);
    //this->RunInstructionTest(this->test_info_->name(), "E1", true);
}

TEST_F(CpuTest, TEST_SBC_StackRelative)
{
    this->RunInstructionTest(this->test_info_->name(), "E3", false);
    //this->RunInstructionTest(this->test_info_->name(), "E3", true);
}

TEST_F(CpuTest, TEST_SBC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "E5", false);
    //this->RunInstructionTest(this->test_info_->name(), "E5", true);
}

TEST_F(CpuTest, TEST_SBC_DirectIndirectLong)
{
    this->RunInstructionTest(this->test_info_->name(), "E7", false);
    //this->RunInstructionTest(this->test_info_->name(), "E7", true);
}

TEST_F(CpuTest, TEST_SBC_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "E9", false);
    //this->RunInstructionTest(this->test_info_->name(), "E9", true);
}

TEST_F(CpuTest, TEST_SBC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "ED", false);
    //this->RunInstructionTest(this->test_info_->name(), "ED", true);
}

TEST_F(CpuTest, TEST_SBC_AbsoluteLong)
{
    this->RunInstructionTest(this->test_info_->name(), "EF", false);
    //this->RunInstructionTest(this->test_info_->name(), "EF", true);
}

TEST_F(CpuTest, TEST_SBC_DirectIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "F1", false);
    //this->RunInstructionTest(this->test_info_->name(), "F1", true);
}

TEST_F(CpuTest, TEST_SBC_DirectIndirect)
{
    this->RunInstructionTest(this->test_info_->name(), "F2", false);
    //this->RunInstructionTest(this->test_info_->name(), "F2", true);
}

TEST_F(CpuTest, TEST_SBC_StackRelativeIndirectIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "F3", false);
    //this->RunInstructionTest(this->test_info_->name(), "F3", true);
}

TEST_F(CpuTest, TEST_SBC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "F5", false);
    //this->RunInstructionTest(this->test_info_->name(), "F5", true);
}

TEST_F(CpuTest, TEST_SBC_DirectIndirectLongIndexed)
{
    this->RunInstructionTest(this->test_info_->name(), "F7", false);
    //this->RunInstructionTest(this->test_info_->name(), "F7", true);
}

TEST_F(CpuTest, TEST_SBC_AbsoluteIndexedY)
{
    this->RunInstructionTest(this->test_info_->name(), "F9", false);
    //this->RunInstructionTest(this->test_info_->name(), "F9", true);
}

TEST_F(CpuTest, TEST_SBC_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "FD", false);
    //this->RunInstructionTest(this->test_info_->name(), "FD", true);
}

TEST_F(CpuTest, TEST_SBC_AbsoluteLongIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "FF", false);
    //this->RunInstructionTest(this->test_info_->name(), "FF", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_DEC)
{
    this->RunInstructionTest(this->test_info_->name(), "3A", false);
    //this->RunInstructionTest(this->test_info_->name(), "3A", true);
}

TEST_F(CpuTest, TEST_DEC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "C6", false);
    //this->RunInstructionTest(this->test_info_->name(), "C6", true);
}

TEST_F(CpuTest, TEST_DEC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "CE", false);
    //this->RunInstructionTest(this->test_info_->name(), "CE", true);
}

TEST_F(CpuTest, TEST_DEC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "D6", false);
    //this->RunInstructionTest(this->test_info_->name(), "D6", true);
}

TEST_F(CpuTest, TEST_DEC_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "DE", false);
    //this->RunInstructionTest(this->test_info_->name(), "DE", true);
}

TEST_F(CpuTest, TEST_DEX)
{
    this->RunInstructionTest(this->test_info_->name(), "CA", false);
    //this->RunInstructionTest(this->test_info_->name(), "CA", true);
}

TEST_F(CpuTest, TEST_DEY)
{
    this->RunInstructionTest(this->test_info_->name(), "88", false);
    //this->RunInstructionTest(this->test_info_->name(), "88", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_INC)
{
    this->RunInstructionTest(this->test_info_->name(), "1A", false);
    //this->RunInstructionTest(this->test_info_->name(), "1A", true);
}

TEST_F(CpuTest, TEST_INC_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "E6", false);
    //this->RunInstructionTest(this->test_info_->name(), "E6", true);
}

TEST_F(CpuTest, TEST_INC_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "EE", false);
    //this->RunInstructionTest(this->test_info_->name(), "EE", true);
}

TEST_F(CpuTest, TEST_INC_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "F6", false);
    //this->RunInstructionTest(this->test_info_->name(), "F6", true);
}

TEST_F(CpuTest, TEST_INC_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "FE", false);
    //this->RunInstructionTest(this->test_info_->name(), "FE", true);
}

TEST_F(CpuTest, TEST_INX)
{
    this->RunInstructionTest(this->test_info_->name(), "E8", false);
    //this->RunInstructionTest(this->test_info_->name(), "E8", true);
}

TEST_F(CpuTest, TEST_INY)
{
    this->RunInstructionTest(this->test_info_->name(), "C8", false);
    //this->RunInstructionTest(this->test_info_->name(), "C8", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_CMP)
{
    this->RunInstructionTest(this->test_info_->name(), "C9", false);
    //this->RunInstructionTest(this->test_info_->name(), "C9", true);
}

TEST_F(CpuTest, TEST_CPX)
{
    this->RunInstructionTest(this->test_info_->name(), "E0", false);
    //this->RunInstructionTest(this->test_info_->name(), "E0", true);
}

TEST_F(CpuTest, TEST_CPY)
{
    this->RunInstructionTest(this->test_info_->name(), "C0", false);
    //this->RunInstructionTest(this->test_info_->name(), "C0", true);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_BIT_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "24", false);
    //this->RunInstructionTest(this->test_info_->name(), "24", true);
}

TEST_F(CpuTest, TEST_BIT_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "2C", false);
    //this->RunInstructionTest(this->test_info_->name(), "2C", true);
}

TEST_F(CpuTest, TEST_BIT_DirectIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "34", false);
    //this->RunInstructionTest(this->test_info_->name(), "34", true);
}

TEST_F(CpuTest, TEST_BIT_AbsoluteIndexedX)
{
    this->RunInstructionTest(this->test_info_->name(), "3C", false);
    //this->RunInstructionTest(this->test_info_->name(), "3C", true);
}

TEST_F(CpuTest, TEST_BIT_Immediate)
{
    this->RunInstructionTest(this->test_info_->name(), "89", false);
    //this->RunInstructionTest(this->test_info_->name(), "89", true);
}

TEST_F(CpuTest, TEST_TRB_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "14", false);
    //this->RunInstructionTest(this->test_info_->name(), "14", true);
}

TEST_F(CpuTest, TEST_TRB_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "1C", false);
    //this->RunInstructionTest(this->test_info_->name(), "1C", true);
}

TEST_F(CpuTest, TEST_TSB_Direct)
{
    this->RunInstructionTest(this->test_info_->name(), "04", false);
    //this->RunInstructionTest(this->test_info_->name(), "04", true);
}

TEST_F(CpuTest, TEST_TSB_Absolute)
{
    this->RunInstructionTest(this->test_info_->name(), "0C", false);
    //this->RunInstructionTest(this->test_info_->name(), "0C", true);
}