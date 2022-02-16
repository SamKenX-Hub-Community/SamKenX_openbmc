// Copyright 2021-present Facebook. All Rights Reserved.
#include "ModbusCmds.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace rackmon;

TEST(ReadHoldingRegisters, Req) {
  ReadHoldingRegistersReq msg(0x1, 0x1234, 0x20);
  Encoder::encode(msg);
  // addr(1) = 0x01,
  // func(1) = 0x03,
  // reg_off(2) = 0x1234,
  // reg_cnt(2) = 0x0020,
  // crc(2) (Pre-computed)
  EXPECT_EQ(msg, 0x01031234002000a4_M);
  EXPECT_EQ(msg.addr, 0x1);
  uint16_t crc;
  msg >> crc;
  EXPECT_EQ(crc, 0xa4);
}

TEST(ReadHoldingRegisters, GoodResp) {
  std::vector<uint16_t> vec(3);
  std::vector<uint16_t> exp{0x1122, 0x3344, 0x5566};
  ReadHoldingRegistersResp msg(0xa, vec);
  EXPECT_EQ(msg.len, 11);
  // addr(1) = 0x0a,
  // func(1) = 0x03,
  // bytes(1) = 0x06,
  // regs(3*2) = 0x1122, 0x3344, 0x5566
  // crc(2) = 0x5928 (pre-computed)
  msg.Msg::operator=(0x0a03061122334455665928_M);
  Encoder::decode(msg);
  EXPECT_EQ(vec, exp);
}

TEST(ReadHoldingRegisters, BadCRCResp) {
  std::vector<uint16_t> vec(3);
  ReadHoldingRegistersResp msg(0xa, vec);
  EXPECT_EQ(msg.len, 11);
  // addr(1) = 0x0a,
  // func(1) = 0x03,
  // bytes(1) = 0x06,
  // regs(3*2) = 0x1122, 0x3344, 0x5566
  // crc(2) = 0x5929 (should be 0x5928)
  msg.Msg::operator=(0x0a03061122334455665929_M);
  EXPECT_THROW(Encoder::decode(msg), CRCError);
}

TEST(ReadHoldingRegisters, BadAddrResp) {
  std::vector<uint16_t> vec(3);
  ReadHoldingRegistersResp msg(0xa, vec);
  EXPECT_EQ(msg.len, 11);
  // Good CRC, bad Function
  // addr(1) = 0x0b
  // func(1) = 0x03
  // bytes(1) = 0x06,
  // regs(3*2) = 0x1122, 0x3344, 0x5566
  msg.Msg::operator=(0x0b0306112233445566_EM);
  EXPECT_THROW(Encoder::decode(msg), BadResponseError);
}

TEST(ReadHoldingRegisters, BadFuncResp) {
  std::vector<uint16_t> vec(3);
  ReadHoldingRegistersResp msg(0xa, vec);
  EXPECT_EQ(msg.len, 11);
  // Good CRC, bad Function
  // addr(1) = 0x0a,
  // func(1) = 0x04 (should be 0x03),
  // bytes(1) = 0x06,
  // regs(3*2) = 0x1122, 0x3344, 0x5566
  // crc(2) = 0x18ce
  msg.Msg::operator=(0x0a040611223344556618ce_M);
  EXPECT_THROW(Encoder::decode(msg), BadResponseError);
}

TEST(ReadHoldingRegisters, BadBytesResp) {
  std::vector<uint16_t> vec(3);
  ReadHoldingRegistersResp msg(0xa, vec);
  EXPECT_EQ(msg.len, 11);
  // Good CRC, bad byte count
  // addr(1) = 0x0a,
  // func(1) = 0x03,
  // bytes(1) = 0x04 (should be 6),
  // regs(3*2) = 0x1122, 0x3344, 0x5566
  // crc(2) = 0x7ae8
  msg.Msg::operator=(0x0a03041122334455667ae8_M);
  EXPECT_THROW(Encoder::decode(msg), BadResponseError);
}

TEST(WriteSingleRegister, Req) {
  WriteSingleRegisterReq msg(0x1, 0x1234, 0x5678);
  Encoder::encode(msg);
  // addr(1) 0x01
  // func(1) 0x06
  // reg_off(2) 0x1234
  // val(2) 0x5678
  EXPECT_EQ(msg, 0x010612345678_EM);
  // we have already tested CRC, just ensure decode
  // does not throw.
  Encoder::decode(msg);
}

TEST(WriteSingleRegister, Resp) {
  WriteSingleRegisterResp msg(0x1, 0x1234);
  EXPECT_EQ(msg.len, 8);
  // addr(1) 0x01
  // func(1) 0x06
  // reg_off(2) 0x1234
  // val(2) 0x5678
  msg.Msg::operator=(0x010612345678_EM);
  Encoder::decode(msg);
  EXPECT_EQ(msg.writtenValue(), 0x5678);
}

TEST(WriteSingleRegister, RespSelfTest) {
  WriteSingleRegisterResp msg(0x1, 0x1234, 0x5678);
  EXPECT_EQ(msg.len, 8);
  // addr(1) 0x01
  // func(1) 0x06
  // reg_off(2) 0x1234
  // val(2) 0x5678
  msg.Msg::operator=(0x010612345678_EM);
  Encoder::decode(msg);
}

TEST(WriteSingleRegister, RespSelfTestFail) {
  WriteSingleRegisterResp msg(0x1, 0x1234, 0x5678);
  EXPECT_EQ(msg.len, 8);
  // addr(1) 0x01
  // func(1) 0x06
  // reg_off(2) 0x1234
  // val(2) 0x5678
  msg.Msg::operator=(0x010612345679_EM);
  EXPECT_THROW(Encoder::decode(msg), BadResponseError);
}

TEST(WriteMultipleRegisters, Req1Reg) {
  WriteMultipleRegistersReq msg(0x1, 0x1234);
  // Check if we avoid writing nothing.
  EXPECT_THROW(Encoder::encode(msg), std::underflow_error);
  uint16_t data = 0x5678;
  msg << data;
  Encoder::encode(msg);
  // addr(1) 0x01,
  // func(1) 0x10,
  // reg_off(2) 0x1234,
  // reg_cnt(2) 0x0001,
  // bytes(1) 0x02,
  // val(2) 0x5678
  EXPECT_EQ(msg, 0x011012340001025678_EM);
  // we have already tested CRC, just ensure decode
  // does not throw.
  Encoder::decode(msg);
}

TEST(WriteMultipleRegisters, Req2Reg) {
  WriteMultipleRegistersReq msg(0x1, 0x1234);
  // Check if we avoid writing nothing.
  EXPECT_THROW(Encoder::encode(msg), std::underflow_error);
  uint16_t data1 = 0x5678;
  uint16_t data2 = 0x123;
  msg << data1 << data2;
  Encoder::encode(msg);
  // addr(1) 0x01,
  // func(1) 0x10,
  // reg_off(2) 0x1234,
  // reg_cnt(2) 0x0002,
  // bytes(1) 0x04,
  // val(2*2) 0x5678 0x0123
  EXPECT_EQ(msg, 0x0110123400020456780123_EM);
  Encoder::decode(msg);
}

TEST(WriteMultipleRegisters, Resp) {
  WriteMultipleRegistersResp msg(0x1, 0x1234, 0x2);
  EXPECT_EQ(msg.len, 8);
  // addr(1) 0x01,
  // func(1) 0x10,
  // reg_off(2) 0x1234,
  // reg_cnt(2) 0x0002,
  msg.Msg::operator=(0x011012340002_EM);
  Encoder::decode(msg);
}

TEST(ReadFileRecord, Req) {
  std::vector<FileRecord> record(2);
  record[0].data.resize(2);
  record[0].fileNum = 4;
  record[0].recordNum = 1;
  record[1].data.resize(2);
  record[1].fileNum = 3;
  record[1].recordNum = 9;
  ReadFileRecordReq req(4, record);
  Encoder::encode(req);
  // Mimick Page 33, (Adds addr)
  // https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
  EXPECT_EQ(req, 0x04140e0600040001000206000300090002_EM);
}

TEST(ReadFileRecord, Resp) {
  std::vector<FileRecord> record(2);
  record[0].data.resize(2);
  record[1].data.resize(2);
  ReadFileRecordResp msg(4, record);
  EXPECT_EQ(msg.len, 17);
  // Mimick Page 33 (Adds addr to resp)
  // https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
  msg.Msg::operator=(0x04140C05060DFE0020050633CD0040_EM);
  Encoder::decode(msg);
}
