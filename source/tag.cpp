#include "tag.hpp"

#include "utils.hpp"

namespace startgit
{

tag::tag(const git2wrap::tag& tagg)
    : m_name(tagg.get_name())
    , m_author(tagg.get_tagger().get_name())
    , m_time(long_to_string(tagg.get_tagger().get_time().time))
{
}

}  // namespace startgit
