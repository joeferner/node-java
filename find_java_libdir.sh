#!/usr/bin/env bash

set -eu

error() {
  echo "error: $*" >&2
  exit 1
}

main () {
  local target_arch=$1
  local os=$2

  echo "$target_arch $os" > /tmp/a

  local java_home full_java_version java_version
  java_home=$(node findJavaHome.js)
  full_java_version=$(${java_home}/bin/java -version 2>&1 | sed -e 's/.*version "\(.*\)"\(.*\)/\1/; 1q')

  if [[ "${full_java_version}" = "1."* ]]
  then
    java_version=$(echo "${full_java_version}" | sed -e 's/1\.\([0-9]*\)\(.*\)/\1/; 1q')
  else
    java_version=$(echo "${full_java_version}" | sed -e 's/\([0-9]*\)\(.*\)/\1/; 1q')
  fi

  local lib_dir
  if [[ "${java_version}" =~ "(6|7|8)" ]]; then
    lib_dir="${java_home}/jre/lib"
  else
    lib_dir="${java_home}/lib"
  fi

  if [[ "${os}" == "linux" && "${target_arch}" == "arm" ]]; then
    if [[ -d ${lib_dir}/arm/classic ]]; then echo ${lib_dir}/arm/classic; else echo ${lib_dir}/arm/server; fi
  elif [[ "${os}" == "linux" && "${target_arch}" == "ia32" ]]; then
    if [[ -d ${lib_dir}/i386/classic ]]; then echo ${lib_dir}/i386/classic; else echo ${lib_dir}/i386/server; fi
  elif [[ "${os}" == "linux" && "${target_arch}" == "x64" ]]; then
    if [[ -d ${lib_dir}/amd64/classic ]]; then echo ${lib_dir}/amd64/classic; else echo ${lib_dir}/amd64/server; fi
  elif [[ "${os}" == "linux" && ( "${target_arch}" == "s390x" || "${target_arch}" == "s390" ) ]]; then
    if [[ -d ${lib_dir}/s390x/classic ]]; then echo ${lib_dir}/s390x/classic; else echo ${lib_dir}/s390/classic; fi
  elif [[ "${os}" == "linux" && ( "${target_arch}" == "ppc64" || "${target_arch}" == "ppc" ) ]]; then
    if [[ -d ${lib_dir}/ppc64/classic ]]; then echo ${lib_dir}/ppc64/classic; fi
  elif [[ "${os}" == "mac" ]]; then
    echo "${lib_dir}/server"
  else
    local arch
    if [[ "${target_arch}" =~ (32|386) ]]; then
        arch=i386
    else
        arch=amd64
    fi
    if [[ "${os}" == "solaris" ]]; then
      echo "${lib_dir}/${arch}/server"
    elif [[ "${os}" == "freebsd" ]]; then
      echo "${lib_dir}/${arch}/server"
    elif [[ "${os}" == "openbsd" ]]; then
      echo "${lib_dir}/${arch}/server"
    else
      error "unknown lib dir"
    fi
  fi
}

main "$@"