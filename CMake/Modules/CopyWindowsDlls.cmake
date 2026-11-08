#
# copy_windows_dlls (<bin dir> <plugin dir> <objdump> <search dir>...)
#
# Install-time helper for Windows builds cross-compiled on another OS
# (e.g. MXE on Linux), used instead of BundleUtilities' fixup_bundle(),
# which can't tell Windows system DLLs from missing ones off Windows and
# fails on them (e.g. "target 'd3d11.dll' does not exist").
#
# Starting from every .exe/.dll under <bin dir> and <plugin dir>, reads
# each file's imports with <objdump> -p and copies every imported DLL that
# is found in one of the <search dir>s into <bin dir>, then scans the
# copies too. Imports found in no search dir are taken to be Windows system
# DLLs and left out; but if such a DLL looks like a library we should have
# shipped (lib*, Qt5*, *fftw*, *hamlib*, *gfortran*, *gomp*, *usb*), it's an
# error, as the program would not start without it.
#
# DLLs loaded at run time rather than imported (e.g. OpenSSL, which Qt
# loads for HTTPS) aren't found by the import scan: list them as glob
# patterns in the variable copy_windows_dlls_extra (e.g. "libssl-*.dll").
# Each match in the search dirs is bundled (and scanned) as well.
#
function (copy_windows_dlls bin_dir plugin_dir objdump)
  set (search_dirs ${ARGN})

  # index the DLLs available for bundling: lowercase name -> path
  set (avail_names)
  foreach (dir ${search_dirs})
    file (GLOB dlls "${dir}/*.dll")
    foreach (dll ${dlls})
      get_filename_component (name "${dll}" NAME)
      string (TOLOWER "${name}" lname)
      list (FIND avail_names "${lname}" idx)
      if (idx EQUAL -1)
        list (APPEND avail_names "${lname}")
        set ("avail_${lname}" "${dll}")
      endif ()
    endforeach ()
  endforeach ()

  # DLLs already in the bin dir (installed by the project itself)
  set (present)
  file (GLOB existing "${bin_dir}/*.dll")
  foreach (dll ${existing})
    get_filename_component (name "${dll}" NAME)
    string (TOLOWER "${name}" lname)
    list (APPEND present "${lname}")
  endforeach ()

  # DLLs loaded at run time, bundled explicitly
  foreach (pattern ${copy_windows_dlls_extra})
    set (found FALSE)
    foreach (dir ${search_dirs})
      file (GLOB matches "${dir}/${pattern}")
      foreach (dll ${matches})
        get_filename_component (name "${dll}" NAME)
        string (TOLOWER "${name}" lname)
        list (FIND present "${lname}" pidx)
        if (pidx EQUAL -1)
          file (COPY "${dll}" DESTINATION "${bin_dir}")
          message (STATUS "Bundled ${name} (loaded at run time)")
          list (APPEND present "${lname}")
        endif ()
        set (found TRUE)
      endforeach ()
    endforeach ()
    if (NOT found)
      message (FATAL_ERROR "copy_windows_dlls: no ${pattern} in ${search_dirs}")
    endif ()
  endforeach ()

  file (GLOB_RECURSE queue "${bin_dir}/*.exe" "${bin_dir}/*.dll" "${plugin_dir}/*.dll")
  set (seen)
  set (missing)
  list (LENGTH queue n)
  while (n GREATER 0)
    list (GET queue 0 pe)
    list (REMOVE_AT queue 0)
    execute_process (COMMAND "${objdump}" -p "${pe}"
      RESULT_VARIABLE rv OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if (NOT rv EQUAL 0)
      message (FATAL_ERROR "copy_windows_dlls: ${objdump} -p ${pe} failed: ${err}")
    endif ()
    string (REGEX MATCHALL "DLL Name: [^\n\r]+" imports "${out}")
    foreach (line ${imports})
      string (REGEX REPLACE "^DLL Name: *" "" name "${line}")
      string (STRIP "${name}" name)
      string (TOLOWER "${name}" lname)
      list (FIND seen "${lname}" idx)
      if (idx EQUAL -1)
        list (APPEND seen "${lname}")
        list (FIND present "${lname}" pidx)
        if (NOT pidx EQUAL -1)
          # already shipped
        elseif (DEFINED "avail_${lname}")
          set (src "${avail_${lname}}")
          get_filename_component (srcname "${src}" NAME)
          file (COPY "${src}" DESTINATION "${bin_dir}")
          message (STATUS "Bundled ${srcname}")
          list (APPEND present "${lname}")
          list (APPEND queue "${bin_dir}/${srcname}")
        elseif (lname MATCHES "^(lib|qt5)|fftw|hamlib|gfortran|gomp|usb")
          list (APPEND missing "${name}")
        endif ()
      endif ()
    endforeach ()
    list (LENGTH queue n)
  endwhile ()

  if (missing)
    message (FATAL_ERROR "copy_windows_dlls: needed DLLs not found in ${search_dirs}: ${missing}")
  endif ()
endfunction ()
