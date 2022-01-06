# Mirroring the Class GitHub Repository

This semester many class assignments are hosted on GitHub in the following
repository:

```
https://github.com/cdeccio/byu-cs324-w2022
```

Individual assignments are in subfolders.  Within the folder for every assignment
is a `README.md` file, containing the description for the assignment.  The
folder also contains other files that are part of the assignment.  The
description of the assignment is often best viewed with a Web browser.  The
accompanying files can be downloaded directly from the folder, with a Web
browser. However, in order to more easily download files associated with
assignments, keep them up-to-date, and simultaneously version your own code
developed for class, we suggest mirroring the class GitHub repository.


## Create a Mirrored Version of the Class Repository

This is a one-time process to create and configure your own private repository
for referencing and committing changes, which repository is a mirror of the
upstream class repository.

Throughout these steps, we will refer to the official class repository as the
"upstream" repository and your own repository as the "new" repository.

 1. Create new repository on github. Follow steps 1 through 6 in the
    [official documentation](https://docs.github.com/en/get-started/quickstart/create-a-repo#create-a-repository),
    adhering to the following:
    - Create the repository under your user ("username"), and name the
      repository `byu-cs324-w2022` (Step 2).
    - Make sure the visibility of the repository is _Private_ (Step 4).
    - Do _not_ check the box "Initialize this repository with a README" (Step 5).
   
 2. Clone the upstream repository by running the following from the
    terminal:
    ```
    git clone --bare https://github.com/cdeccio/byu-cs324-w2022 upstream-repo
    ```

 3. Push a mirror of the upstream repository to the new repository, which you
    have just created:
    ```
    cd upstream-repo
    git push --mirror ssh://git@github.com/username/byu-cs324-w2022
    ```
    (substitute "username" with your GitHub username)

 4. Remove your clone of the upstream repository.
    ```
    cd ../
    rm -rf upstream-repo
    ```

 5. Clone your new repository, which is now a mirror of the upstream repository:
    ```
    git clone ssh://git@github.com/username/byu-cs324-w2022
    ```
    (substitute "username" with your GitHub username)

 6. Add the upstream repository to your clone:
    ```
    cd byu-cs324-w2022
    git remote add upstream ssh://git@github.com/cdeccio/byu-cs324-w2022
    git remote -v
    ```

## Update Your Mirrored Repository from the Upstream

Do this every time you would like to pull down the changes from the upstream
repository and integrate them into your own repository:

 1. Pull down the latest changes from both your repository and the upstream:
    ```
    git fetch --all
    ```
 2. Stash (save aside) any uncommitted changes that you might have locally in
    your clone:
    ```
    git stash
    ```
 3. Merge in the changes from the upstream repository:
    ```
    git merge upstream/master
    ```
 4. Merge back in any uncommitted changes that were stashed:
    ```
    git stash pop
    ```
 5. Push out the locally merged changes to your repository:
    ```
    git push
    ```

## Commit and push local changes to private repo:

Do this every time you want to commit changes to the clone of your repository
and push them out to the repository:

 1. Commit any local changes that you've made (i.e., in your own development):
    ```
    git commit ...
    ```
    (replace "..." with the names of any files or directories that have changes)
 2. Push out your local commits to your repository:
    ```
    git push
    ```
