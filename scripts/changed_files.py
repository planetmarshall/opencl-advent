from argparse import ArgumentParser
import json
import os

from git import Repo



def all_files(compilation_database_file_name):
    with open(compilation_database_file_name, "r") as fp:
        db = json.load(fp)
        for record in db:
            yield record["file"]


def changed_files(working_dir, target="main"):
    repo = Repo(working_dir)
    diff = repo.commit(target).diff(repo.head)
    for change in diff:
        yield os.path.join(working_dir, change.a_path)


def main():
    def default_working_dir():
        return os.path.dirname(os.path.dirname(__file__))

    parser = ArgumentParser()
    parser.add_argument("database")
    parser.add_argument("--working-dir", default=default_working_dir())
    parser.add_argument("--target", default="main")

    args = parser.parse_args()
    changes = set(changed_files(args.working_dir, target=args.target))
    compiled_files = set(all_files(args.database))

    changed_compiled_files = changes.intersection(compiled_files)
    print(" ".join(changed_compiled_files))


if __name__ == "__main__":
    main()
