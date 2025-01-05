#include <format>
#include <iostream>

#include <git2.h>
#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <git2wrap/object.hpp>
#include <git2wrap/repository.hpp>
#include <git2wrap/revwalk.hpp>
#include <git2wrap/signature.hpp>

int main()
{
  try {
    using namespace git2wrap;  // NOLINT

    const libgit2 libgit;

    repository repo = repository::open(
        "../maintain/stellar", GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr);

    for (auto it = repo.branch_begin(GIT_BRANCH_LOCAL); it != repo.branch_end();
         ++it)
    {
      const object obj = repo.revparse(it->get_name().c_str());
      std::cout << it->get_name() << " " << obj.get_id() << '\n';

      revwalk rwalk(repo);
      rwalk.push(obj.get_id());

      while (const auto commit = rwalk.next()) {
        std::cout << std::format("{}: {} ({})\n",
                                 commit.get_time(),
                                 commit.get_summary(),
                                 commit.get_author().get_name());
      }

      break;
    }

  } catch (const git2wrap::error& err) {
    std::cerr << std::format("({}:{}) Error {}/{}: {}\n",
                             err.get_file(),
                             err.get_line(),
                             err.get_error(),
                             err.get_klass(),
                             err.get_message());
  }

  return 0;
}
