#include "mytinyhttpd/net/buffer.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace mytinyhttpd {

namespace net {

TEST(BufferTest, AppendRetrieveTest) {
  Buffer buf;
  ASSERT_EQ(buf.readable_bytes(), 0LU);
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize);
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend);

  const std::string str1(200, 'x');
  buf.Append(str1);
  ASSERT_EQ(buf.readable_bytes(), str1.size());
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize - str1.size());
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend);

  const std::string str2 = buf.RetrieveAsString(50);
  ASSERT_EQ(str2.size(), 50LU);
  ASSERT_EQ(buf.readable_bytes(), str1.size() - str2.size());
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize - str1.size());
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend + str2.size());
  ASSERT_EQ(str2, str1.substr(0, 50));

  buf.Append(str1);
  ASSERT_EQ(buf.readable_bytes(), 2 * str1.size() - str2.size());
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize - 2 * str1.size());
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend + str2.size());

  const std::string str3 = buf.RetrieveAllAsString();
  ASSERT_EQ(str3.size(), 350LU);
  ASSERT_EQ(buf.readable_bytes(), 0LU);
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize);
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend);
  ASSERT_EQ(str3, std::string(350, 'x'));
}

TEST(BufferTest, GrowTest) {
  Buffer buf;
  buf.Append(std::string(400, 'y'));
  ASSERT_EQ(buf.readable_bytes(), 400LU);
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize - 400);

  buf.Retrieve(50);
  ASSERT_EQ(buf.readable_bytes(), 350LU);
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize - 400);
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend + 50);

  buf.Append(std::string(1000, 'z'));
  ASSERT_EQ(buf.readable_bytes(), 1350LU);
  ASSERT_EQ(buf.writable_bytes(), 58);
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend);

  buf.RetrieveAll();
  ASSERT_EQ(buf.readable_bytes(), 0LU);
  ASSERT_EQ(buf.writable_bytes(), 1408LU);
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend);
}

TEST(BufferTest, PrependTest) {
  Buffer buf;
  buf.Append(std::string(200, 'y'));
  ASSERT_EQ(buf.readable_bytes(), 200);
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize - 200);
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend);

  int x = 0;
  buf.Prepend(&x, sizeof x);
  ASSERT_EQ(buf.readable_bytes(), 204);
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize - 200);
  ASSERT_EQ(buf.prependable_bytes(), Buffer::kCheapPrepend - 4);
}

TEST(BufferTest, ReadIntTest) {
  Buffer buf;
  buf.Append("HTTP");

  ASSERT_EQ(buf.readable_bytes(), 4LU);
  ASSERT_EQ(buf.PeekInt8(), 'H');
  int top16 = buf.PeekInt16();
  ASSERT_EQ(top16, 'H' * 256 + 'T');
  ASSERT_EQ(buf.PeekInt32(), top16 * 65536 + 'T' * 256 + 'P');

  ASSERT_EQ(buf.ReadInt8(), 'H');
  ASSERT_EQ(buf.ReadInt16(), 'T' * 256 + 'T');
  ASSERT_EQ(buf.ReadInt8(), 'P');
  ASSERT_EQ(buf.readable_bytes(), 0LU);
  ASSERT_EQ(buf.writable_bytes(), Buffer::kInitialSize);

  buf.AppendInt8(-1);
  buf.AppendInt16(-2);
  buf.AppendInt32(-3);
  ASSERT_EQ(buf.readable_bytes(), 7LU);
  ASSERT_EQ(buf.ReadInt8(), static_cast<int8_t>(-1));
  ASSERT_EQ(buf.ReadInt16(), static_cast<int64_t>(-2));
  ASSERT_EQ(buf.ReadInt32(), static_cast<int32_t>(-3));
}

TEST(BufferTest, FindEolTest) {
  Buffer buf;
  buf.Append(std::string(100000, 'x'));
  const char* null = NULL;
  ASSERT_EQ(buf.FindEol(), null);
  ASSERT_EQ(buf.FindEol(buf.Peek() + 90000), null);
}

void output(Buffer&& buf, const void* inner) {
  Buffer new_buf(std::move(buf));
  ASSERT_EQ(inner, new_buf.Peek());
}

TEST(BufferTest, MoveTest) {
  Buffer buf;
  buf.Append("mytinyhttpd", 5);
  const void* inner = buf.Peek();
  output(std::move(buf), inner);
}

}  // namespace net

}  // namespace mytinyhttpd
