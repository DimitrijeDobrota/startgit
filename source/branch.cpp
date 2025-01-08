#include "branch.hpp"

namespace startgit
{

branch::branch(git2wrap::branch brnch)
    : m_branch(std::move(brnch))
    , m_name(m_branch.get_name())
{
}

}  // namespace startgit
