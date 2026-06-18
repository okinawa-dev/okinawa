#include "docsgen.hpp"

namespace docsgen {

bool parseFrontMatter(const std::string &content, FrontMatter &fmOut,
                      std::string &bodyOut) {
  (void)fmOut;
  bodyOut = content;
  return false;
}

std::string outputPathFor(const std::string &srcRelPath) { return srcRelPath; }

int sectionRank(const std::string &section) {
  (void)section;
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
