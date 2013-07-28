#pragma once

#include <string>
#include <memory>
#include <set>

namespace bold
{
  typedef unsigned char uchar;

  class MotionScriptPage;
  class MotionScriptPage;

  /** Represents a collection of playable, scripted motions.
   *
   * The file has 256 pages at 512 bytes per page, totalling 131,072 bytes.
   */
  class MotionScriptFile
  {
  public:
    /// Page numbers run from 1 to 255
    static const uchar MAX_PAGE_ID = 255;

    static std::shared_ptr<MotionScriptFile> loadFromBinaryFile(std::string const& filePath);
    static std::shared_ptr<MotionScriptFile> loadFromJsonFile(std::string const& filePath);

    MotionScriptFile();

    MotionScriptFile(std::shared_ptr<MotionScriptPage> d_pages[(ushort)MAX_PAGE_ID + 1]);

    bool saveToBinaryFile(std::string const& filePath) const;
    bool saveToJsonFile(std::string const& filePath) const;

    /// Gets the first page with the specified name.
    std::shared_ptr<MotionScriptPage> getPageByName(std::string const& pageName) const;

    /// Gets the page
    std::shared_ptr<MotionScriptPage> getPageByIndex(uchar pageIndex) const;

    /** Returns the set of page names found in this MotionScriptFile.
     *
     * Not all pages have names, and there may be multiple pages with the same name.
     */
    std::set<std::string> getPageNames() const;

  private:
    std::shared_ptr<MotionScriptPage> d_pages[(ushort)MAX_PAGE_ID + 1];
  };
}
