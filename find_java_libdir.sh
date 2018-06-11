#!/usr/bin/env bash

set -eu

error() {
  echo "error: $*" >&2
  exit 1
}

main () {
  local target_arch=$1
  local os=$2

  local java_home full_java_version java_version
  java_home=$(node findJavaHome.js)
  full_java_version=$(${java_home}/bin/java -version 2>&1 | grep version | sed -e 's/.*version "\(.*\)"\(.*\)/\1/; 1q')

  if [[ "${full_java_version}" = "1."* ]]
  then
    java_version=$(echo "${full_java_version}" | sed -e 's/1\.\([0-9]*\)\(.*\)/\1/; 1q')
  else
    java_version=$(echo "${full_java_version}" | sed -e 's/\([0-9]*\)\(.*\)/\1/; 1q')
  fi

  local jre_dir
  if [[ "${java_version}" =~ (6|7|8) ]]; then
    jre_dir="${java_home}/jre/lib"
  else
    jre_dir="${java_home}/lib"
  fi

  local lib_dir=""
  if [[ "${os}" == "linux" && ! "${java_version}" =~ (6|7|8) ]]; then
    # no arch on JDK 9+
    lib_dir="${jre_dir}/server"
  elif [[ "${os}" == "linux" && "${target_arch}" == "arm" ]]; then
    if [[ -d ${jre_dir}/arm/classic ]]; then lib_dir="${jre_dir}"/arm/classic; else lib_dir="${jre_dir}"/arm/server; fi
  elif [[ "${os}" == "linux" && "${target_arch}" == "ia32" ]]; then
    if [[ -d ${jre_dir}/i386/classic ]]; then lib_dir="${jre_dir}"/i386/classic; else lib_dir="${jre_dir}"/i386/server; fi
  elif [[ "${os}" == "linux" && "${target_arch}" == "x64" ]]; then
    if [[ -d ${jre_dir}/amd64/classic ]]; then lib_dir="${jre_dir}"/amd64/classic; else lib_dir="${jre_dir}"/amd64/server; fi
  elif [[ "${os}" == "linux" ]] && [[ "${target_arch}" == "s390x" || "${target_arch}" == "s390" ]]; then
    if [[ -d ${jre_dir}/s390x/classic ]]; then lib_dir="${jre_dir}"/s390x/classic; else lib_dir="${jre_dir}"/s390/classic; fi
  elif [[ "${os}" == "linux" ]] && [[ "${target_arch}" == "ppc64" || "${target_arch}" == "ppc" ]]; then
    target_arch=`uname -m`
    if [[ -d ${jre_dir}/${target_arch}/classic ]]; then lib_dir="${jre_dir}"/${target_arch}/classic; else lib_dir="${jre_dir}"/${target_arch}/server; fi
  elif [[ "${os}" == "mac" ]]; then
    lib_dir="${jre_dir}/server"
  else
    local arch
    if [[ "${target_arch}" =~ (32|386) ]]; then
        arch=i386
    else
        arch=amd64
    fi
    if [[ "${os}" == "solaris" ]]; then
      lib_dir="${jre_dir}/${arch}/server"
    elif [[ "${os}" == "freebsd" ]]; then
      lib_dir="${jre_dir}/${arch}/server"
    elif [[ "${os}" == "openbsd" ]]; then
      lib_dir="${jre_dir}/${arch}/server"
    fi
  fi

  if [[ -z "${lib_dir}" ]]; then
    error "Can't find lib dir for ${os} ${target_arch}, java home: ${java_home}"
  fi
  echo "${lib_dir}"
}

main "$@"
