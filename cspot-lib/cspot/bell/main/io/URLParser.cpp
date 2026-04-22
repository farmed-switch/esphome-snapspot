#include "URLParser.h"
#include <cstring>

namespace bell {

#ifdef BELL_DISABLE_REGEX
void URLParser::parse(const char* url, std::vector<std::string>& match) {
  match[0] = url;
  char scratch[512];

  if (sscanf(url, "%[^:]:/", scratch) > 0)
    match[1] = scratch;

  if (sscanf(url, "htt%*[^:]://%512[^/#?]", scratch) > 0)
    match[2] = scratch;

  url = strstr(url, match[2].c_str()) + match[2].size();
  if (sscanf(url, "/%512[^?]", scratch) > 0)
    match[3] = scratch;
  else if (*url && *url != '?' && *url != '#')
    url++;

  if (match[3].size())
    url += match[3].size() + 1;
  if (sscanf(url, "?%512[^#]", scratch) > 0)
    match[4] = scratch;

  if (match[4].size())
    url += match[4].size() + 1;
  if (sscanf(url, "#%512s", scratch) > 0)
    match[5] = scratch;

  match[3] = "/" + match[3];
  if (match[4].size())
    match[4] = "?" + match[4];

  if (match[1].size() == 0 || match[2].size() == 0)
    match.clear();
}
#else
const std::regex URLParser::urlParseRegex = std::regex(
    "^(?:([^:/?#]+):)?(?://([^/?#]*))?([^?#]*)(\\?(?:[^#]*))?(#(?:.*))?");
#endif
}
