# A number of utility functions for SanRazor-clang

# This file is modified from ASAP SanRazor.
# Please see LICENSE.txt for copyright and licensing information.
 
require 'shellwords'

SCRIPT_DIR = File.dirname($0)

# Running external commands
# =========================

class RunExternalCommandError < StandardError
end

def run!(*args)
  kwargs = if args.last.is_a?(Hash) then args.pop else {} end
  $stderr.puts Shellwords.join(args) if $VERBOSE
  # print(args,"\n")
  if not system(*args, kwargs)
    raise RunExternalCommandError, "Command #{args[0]} failed with status #{$?}"
  end
  
end

# Finding stuff in the path
# =========================

# Cross-platform way of finding an executable in the $PATH.
# From https://stackoverflow.com/questions/2108727/which-in-ruby-checking-if-program-exists-in-path-from-ruby
#
#   which('ruby') #=> /usr/bin/ruby
#
def which(cmd)
  exts = ENV['PATHEXT'] ? ENV['PATHEXT'].split(';') : ['']
  ENV['PATH'].split(File::PATH_SEPARATOR).each do |path|
    exts.each { |ext|
      exe = File.join(path, "#{cmd}#{ext}")
      return exe if File.executable? exe
    }
  end
  return nil
end

def find_clang()
  clang = $0.sub(/SanRazor-clang(\+\+)?$/, 'clang\\1')
  raise "cannot find clang" if $0 == clang
  clang
end

def find_opt()
  opt = $0.sub(/SanRazor-clang(\+\+)?$/, 'opt')
  raise "cannot find opt" if $0 == opt
  opt
end

def find_llc()
  llc = $0.sub(/SanRazor-clang(\+\+)?$/, 'llc')
  raise "cannot find llc" if $0 == llc
  llc
end

def find_ar()
  which('ar')
end

def find_SanRazor_lib()
  ["#{SCRIPT_DIR}/../lib/SRPass.dylib",
   "#{SCRIPT_DIR}/../lib/SRPass.so"].find { |f| File.file?(f) }
end


# Transforming file names
# =======================

def mangle(name, ext, new_ext)
  raise "name does not end in #{ext}: #{name}" unless name.end_with?(ext)
  name.sub(/#{Regexp.escape(ext)}$/, new_ext)
end


# Dealing with compiler arguments
# ===============================

# Retrieves the argument that matches a given pattern. This function tries to
# be slightly smart, knowing special cases for common patterns.
# The 'multiple' parameter can be set to :first, :last or to some index. If
# set, multiple values for the option are allowed, and the corresponding one
# will be returned.
def get_arg(args, pattern, multiple=nil)
  if ['-o', '-MF'].include? pattern
    i = args.index(pattern)
    return i ? args[i + 1] : nil
  end

  if pattern.is_a?(Regexp)
    result = args.find_all { |a| a =~ pattern }
  elsif pattern.end_with?('=')
    regexp = /#{Regexp.escape(pattern)}(.*)/
    result = args.collect { |a| if a =~ regexp then $1 else nil end }.compact
  else
    result = args.find_all { |a| a == pattern }
  end

  raise "more than one argument matching #{pattern}" if result.size > 1 and not multiple
  return result[0] if multiple == :first
  return result[-1] if multiple == :last
  return result[multiple || 0]
end

def remove_arg(args, pattern)
  if pattern.is_a?(Regexp)
    args.reject { |a| a =~ pattern }
  else
    args.reject { |a| a == pattern }
  end
end

# Ensures arg is contained in args. Does not introduce duplicates.
def insert_arg(args, arg)
  if arg =~ /^-g/
    args = remove_arg(args, '-g0')
    return args if args.find { |a| a == '-g' }
  end

  return args if args.find { |a| a == arg }
  [arg] + args
end

# Gets the optimization level, but sanitize it to one of the values that LLC understands
def get_optlevel_for_llc(args)
  # Don't use /^-O.$/ here, because llc only knows numeric levels
  get_arg(args, /^-O[0123]$/, :last) || '-O3'
end
