#!/bin/zsh

student_repo=$1
task_name=$2


final_score=0

function exit_with_code {
  message=$1
  code=$2
  echo "Error: $message"
  echo "Error code: $code"
  echo "Final score: $final_score"
  exit 0
}

git ls-remote --exit-code --heads $student_repo $task_name &> /dev/null
errc=$?
if [ $errc -ne 0 ]
then
  exit_with_code "Your repo don't have branch ${task_name}" 10
fi

echo "Cloning solution repo $student_repo"
git clone -b $task_name $student_repo solution &> /dev/null
errc=$?
if [ $errc -ne 0 ]
then
  exit_with_code "You provided broken link to the repo. Check that you invited techprogchecker to collaborators." 1
fi

sudents_repo_path=$(realpath solution)

echo "Cloning course repo"
git clone "git@github.com:Mrkol/metaprogramming-course.git" course &> /dev/null
errc=$?
if [ $errc -ne 0 ]
then
  exit_with_code "Failed to clone course repository. Is the internet down?" 1
fi

pushd course/tasks &> /dev/null
mkdir build
pushd build &> /dev/null

echo "Configuring the task build"
cmake .. -DREPOSITORY_PATH=${sudents_repo_path} -DTASK=${task_name} -DNOCOMPILE=YES

while IFS=' ' read -A test_block
do
  block_name=${test_block[1]}
  test_names=(${(@s:,:)test_block[2]})
  block_score=${test_block[3]}
  echo "Testing block ${block_name}"
  for test in $test_names
  do
    echo "Building test ${test}"
    cmake --build . --target ${test}
    errc=$?
    if [ -z "${test##*nocompile*}" ]
    then
      if [ $errc -eq 0 ]
      then
        echo "Failed test ${test} from block ${block_name}!"
        echo "Code compiled, while it shouldn't have!"
        block_score=0
      fi
      break
    else
      if [ $errc -ne 0 ]
      then
          echo "Failed test ${test} from block ${block_name}!"
          echo "Code didn't compile!"
          block_score=0
          break
      fi
    fi
    echo "Running test ${test}"
    ctest -R ${test}
    errc=$?
    if [ $errc -ne 0 ]
    then
      echo "Failed test ${test} from block ${block_name}!"
      echo "Encountered a runtime assertion/error!"
      block_score=0
      break
    fi
  done
  if [ $block_score -ne 0 ]
  then
    echo "SUCCESS! Test ${test} passed!"
  fi
  (( final_score += block_score ))
done < "../${task_name}/tests/tests"

echo "Final score: ${final_score}"

popd 2> /dev/null > /dev/null
popd 2> /dev/null > /dev/null
