#include "util.h"
#include "clog.h"
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>

static void trim(std::string &s) {
    // 删除开头空白
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));

    // 删除结尾空白
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// 判断字符是否是句子分隔符（中英文标点）
bool isSentenceSeparator(char32_t c) {
  // 中文标点
  if (c == U'，' || c == U'。' || c == U'？' || c == U'！' || c == U'；') {
    return true;
  }
  // 英文标点
  if (c == U',' || c == U'.' || c == U'?' || c == U'!' | c == U';') {
    return true;
  }
  return false;
}

// 将UTF-8字符串转换为UTF-32字符串
std::u32string utf8ToUtf32(const std::string &utf8_str) {
  std::u32string utf32_str;

  for (size_t i = 0; i < utf8_str.size();) {
    char32_t code_point = 0;
    unsigned char byte = utf8_str[i];

    if ((byte & 0x80) == 0) {
      // 1字节UTF-8
      code_point = byte;
      i += 1;
    } else if ((byte & 0xE0) == 0xC0) {
      // 2字节UTF-8
      if (i + 1 >= utf8_str.size())
        break;
      code_point = ((byte & 0x1F) << 6) | (utf8_str[i + 1] & 0x3F);
      i += 2;
    } else if ((byte & 0xF0) == 0xE0) {
      // 3字节UTF-8
      if (i + 2 >= utf8_str.size())
        break;
      code_point = ((byte & 0x0F) << 12) | ((utf8_str[i + 1] & 0x3F) << 6) |
                   (utf8_str[i + 2] & 0x3F);
      i += 3;
    } else if ((byte & 0xF8) == 0xF0) {
      // 4字节UTF-8
      if (i + 3 >= utf8_str.size())
        break;
      code_point = ((byte & 0x07) << 18) | ((utf8_str[i + 1] & 0x3F) << 12) |
                   ((utf8_str[i + 2] & 0x3F) << 6) | (utf8_str[i + 3] & 0x3F);
      i += 4;
    } else {
      // 无效UTF-8序列
      i += 1;
      continue;
    }

    utf32_str.push_back(code_point);
  }

  return utf32_str;
}

// 将UTF-32字符串转换为UTF-8字符串
std::string utf32ToUtf8(const std::u32string &utf32_str) {
  std::string utf8_str;

  for (char32_t c : utf32_str) {
    if (c <= 0x7F) {
      utf8_str.push_back(static_cast<char>(c));
    } else if (c <= 0x7FF) {
      utf8_str.push_back(static_cast<char>(0xC0 | ((c >> 6) & 0x1F)));
      utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else if (c <= 0xFFFF) {
      utf8_str.push_back(static_cast<char>(0xE0 | ((c >> 12) & 0x0F)));
      utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
      utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else {
      utf8_str.push_back(static_cast<char>(0xF0 | ((c >> 18) & 0x07)));
      utf8_str.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
      utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
      utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    }
  }

  return utf8_str;
}

// 中英文混合拆句函数
std::vector<std::string> splitMixedSentences(std::string &text, bool last) {
  std::vector<std::string> sentences;
  std::u32string utf32_text = utf8ToUtf32(text);

  size_t start = 0;
  auto min_length = [](char32_t c) {
	/*
    if (c == U'。' || c == U'？' || c == U'！') {
      return 1;
    }
    // 英文标点
    if (c == U'.' || c == U'?' || c == U'!') {
      return 1;
    }
	*/
    return 8;
  };

  for (size_t i = 0; i < utf32_text.size(); ++i) {
    if (isSentenceSeparator(utf32_text[i])) {
      // 检查是否达到最小长度要求或者是最后一个句子
      if (i - start >= min_length(utf32_text[i])) {
        std::u32string sentence_utf32(utf32_text.begin() + start,
                                      utf32_text.begin() + i);
		    std::string s = utf32ToUtf8(sentence_utf32);
		    trim(s);
        if (s.size() > 0) {
          sentences.push_back(s);
        }
        start = i + 1;
      }
    }
  }

  // 处理剩余部分（最后一个句子）
  if (start < utf32_text.size()) {
    std::u32string sentence_utf32(utf32_text.begin() + start, utf32_text.end());
    std::string lastSent = utf32ToUtf8(sentence_utf32);
    if (last and lastSent.size() > 0) {
      sentences.push_back(utf32ToUtf8(sentence_utf32));
    } else {
      text = lastSent;
    }
  } else {
    text = "";
  }

  return sentences;
}
