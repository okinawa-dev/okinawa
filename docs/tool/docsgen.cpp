#include "docsgen.hpp"

#include <cstdlib>

namespace {

std::string trim(const std::string &s) {
  std::size_t a = 0;
  std::size_t b = s.size();
  while (a < b && (s[a] == ' ' || s[a] == '\t')) a++;
  while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r'))
    b--;
  return s.substr(a, b - a);
}

}  // namespace

namespace docsgen {

bool parseFrontMatter(const std::string &content, FrontMatter &fmOut,
                      std::string &bodyOut) {
  fmOut = FrontMatter();
  fmOut.navOrder = 0;
  fmOut.hasNavOrder = false;

  std::size_t pos = 0;
  if (content.compare(0, 3, "\xEF\xBB\xBF") == 0) pos = 3;  // skip BOM

  std::size_t firstNl = content.find('\n', pos);
  if (firstNl == std::string::npos) {
    bodyOut = content;
    return false;
  }
  std::string firstLine = trim(content.substr(pos, firstNl - pos));
  if (firstLine != "---") {
    bodyOut = content;
    return false;
  }

  std::size_t lineStart = firstNl + 1;
  while (lineStart <= content.size()) {
    std::size_t nl = content.find('\n', lineStart);
    std::string line = (nl == std::string::npos)
                           ? content.substr(lineStart)
                           : content.substr(lineStart, nl - lineStart);
    if (trim(line) == "---") {
      bodyOut = (nl == std::string::npos) ? "" : content.substr(nl + 1);
      return true;
    }
    std::size_t colon = line.find(':');
    if (colon != std::string::npos) {
      std::string key = trim(line.substr(0, colon));
      std::string val = trim(line.substr(colon + 1));
      if (key == "title") {
        fmOut.title = val;
      } else if (key == "section") {
        fmOut.section = val;
      } else if (key == "nav_order") {
        fmOut.navOrder = std::atoi(val.c_str());
        fmOut.hasNavOrder = true;
      }
    }
    if (nl == std::string::npos) break;
    lineStart = nl + 1;
  }

  bodyOut = content;  // no closing delimiter -> not a valid block
  return false;
}

std::string outputPathFor(const std::string &srcRelPath) {
  const std::string ext = ".md";
  if (srcRelPath.size() >= ext.size() &&
      srcRelPath.compare(srcRelPath.size() - ext.size(), ext.size(), ext) ==
          0) {
    return srcRelPath.substr(0, srcRelPath.size() - ext.size()) + ".html";
  }
  return srcRelPath;
}

int sectionRank(const std::string &section) {
  if (section == "Start") return 0;
  if (section == "Reference") return 1;
  if (section == "Examples") return 2;
  return 1000;
}

std::string buildNav(const std::vector<Page> &pages,
                     const std::string &currentOutPath) {
  (void)pages;
  (void)currentOutPath;
  return "";
}

std::string applyBaseUrl(const std::string &html, const std::string &baseUrl) {
  (void)baseUrl;
  return html;
}

std::string renderTemplate(const std::string &tmpl, const std::string &title,
                           const std::string &nav, const std::string &content,
                           const std::string &baseUrl) {
  (void)title;
  (void)nav;
  (void)content;
  (void)baseUrl;
  return tmpl;
}

std::string renderMarkdown(const std::string &markdown) { return markdown; }

}  // namespace docsgen
