#!/usr/bin/env ruby

# This is a wrapper script around clang, ar, ranlib, etc., to perform the
# different SanRazor compilation steps:
#
# - First step: -SR-init
#   Creates the SanRazor state directory, which contains all the additional files
#   that SanRazor manages throughout a compilation.
#   After SanRazor-init, the software is ready to be compiled.
# - Second step: -SR-opt -SR-opt -san-level=>L0/L1/L2>
#   Prepares for optimized compilation. Running make/ninja again after this
#   should result in an optimized binary.

# This file is part of SanRazor.
# Please see LICENSE.txt for copyright and licensing information.

require 'fileutils'
require 'parallel'
require 'pathname'

require_relative 'SanRazor-clang-utils.rb'

# This class keeps track of the state of the SanRazor compilation. It maintains the
# current state, compilation output files, ...
class SRState
  attr_reader :state_path

  def initialize()
    @state_path = ENV['SR_STATE_PATH']
    
    
    raise "Please set SR_STATE_PATH" unless state_path
    raise "SR_STATE_PATH should be absolute" unless state_path == File.expand_path(state_path)
    raise "SR_STATE_PATH must be an existing folder" unless File.directory?(state_path)
  end

  def self.initialize_state(level)
    ENV['SR_STATE_PATH'] ||= File.realdirpath('Cov')
    FileUtils.cp(ENV['SR_WORK_PATH'], Dir.pwd+"/"+"coverage.sh")
    raise "No coverage.sh!!!" unless File.exist?(ENV['SR_WORK_PATH'])


    if File.exist?(ENV['SR_STATE_PATH'])
      $stderr.puts "Warning: removing old Cov folder #{ENV['SR_STATE_PATH']}" if $VERBOSE
      FileUtils.rm_r(ENV['SR_STATE_PATH'])
    end

    FileUtils.mkdir_p(ENV['SR_STATE_PATH'])

    state = self.new
    state.current_state = :initial
    state.san_level = :level
    state.use_asap = :use_asap
    puts "# Initialized Cov folder in #{state.state_path}. Now run:"
    puts "export SR_STATE_PATH=\"#{state.state_path}\""
  end

  # Create methods that compute paths to state subfolders
  [:coverage, :objects, :costs, :log].each do |dir|
    define_method "#{dir}_path".to_sym do |target|
      target_path = File.expand_path(target)
      target_rel = remove_shared_path_components(target_path, state_path)
      File.join(state_path, dir.to_s, target_rel)
    end

    define_method "#{dir}_directory".to_sym do
      File.join(state_path, dir.to_s)
    end
  end

  # Given two paths, returns the first path with all shared folders removed.
  # For example, /foo/bar/baz, /foo/quu => bar/baz
  #              /foo/bar/baz, /quu => foo/bar/baz
  def remove_shared_path_components(a, b)
    a_path = Pathname.new(a)
    b_path = Pathname.new(b)
    raise "Absolute path expected but #{a} given." unless a_path.absolute?
    raise "Absolute path expected but #{b} given." unless b_path.absolute?

    a_components = a_path.each_filename.to_a
    b_components = b_path.each_filename.to_a

    while a_components[0] == b_components[0]
      a_components.shift
      b_components.shift
    end
    File.join(a_components)
  end

  # Creates the right compiler for the given state
  def create_compiler()
    if current_state == :initial
      SRInitialCompiler.new(self)
    elsif current_state == :optimize
      SROptimizingCompiler.new(self)
    elsif current_state == :sanitizer
      SRSanCompiler.new(self)
    else
      raise "Unknown SR state: #{current_state}"
    end
  end

  def current_state()
    @current_state ||= IO.read(File.join(state_path, "current_state")).chomp.to_sym
  end

  def current_state=(state)
    @current_state = state
    IO.write(File.join(state_path, "current_state"), "#{state}\n")
  end

  def san_level()
    @san_level ||= IO.read(File.join(state_path, "san_level")).chomp.to_sym
  end

  def san_level=(level)
    @san_level = level
    IO.write(File.join(state_path, "san_level"), "#{level}\n")
  end

  def use_asap()
    @use_asap ||= IO.read(File.join(state_path, "use_asap")).chomp.to_sym
  end

  def use_asap=(level)
    @use_asap = level
    IO.write(File.join(state_path, "use_asap"), "#{level}\n")
  end

  def transition(from, to)
    raise "Expected SR state to be '#{from}', but it is '#{current_state}'" unless current_state == from
    yield
    self.current_state = to
  end

end


# This is a base class for executing compilation steps. The default behavior is
# to forward the commands to the original clang/ar/ranlib.
class BaseCompiler
  attr_reader :state

  def initialize(state)
    @state = state
  end

  def exec(cmd)
    command_type = get_command_type(cmd)
    if command_type == :compile
      do_compile(cmd)
    elsif command_type == :link
      do_link(cmd)
    elsif command_type == :ar
      do_ar(cmd)
    elsif command_type == :ranlib
      do_ranlib(cmd)
    else
      raise "invalid command: #{cmd}"
    end
  end

  def get_command_type(cmd)
    if cmd[0] =~ /(?:SanRazor-|\/)clang(?:\+\+)?$/
        if get_arg(cmd, '-c', :first)
            return :compile
        else
            return :link
        end
    elsif cmd[0] =~ /(?:SanRazor-|\/)ar$/
        return :ar
    elsif cmd[0] =~ /(?:SanRazor-|\/)ranlib$/
        return :ranlib
    end
  end

  def do_compile(cmd)
    cmd = [find_clang()] + cmd[1..-1]
    run!(*cmd)
  end
  def do_link(cmd)
    cmd = [find_clang()] + cmd[1..-1]
    run!(*cmd)
  end
  def do_ar(cmd)
    cmd = [find_ar()] + cmd[1..-1]
    run!(*cmd)
  end
  def do_ranlib(cmd)
    cmd = [find_ar(), '-s'] + cmd[1..-1]
    run!(*cmd)
  end
end

# This is the compiler for SanRazor's first stage. It ensures that crucial
# compilation flags are present.
class SRInitialCompiler < BaseCompiler
  def do_compile(cmd)
    clang = find_clang()
    target_name_o = get_arg(cmd, '-o')
    if target_name_o[0] == "."
      target_name = target_name_o[1..-1]
    else
      target_name = target_name_o
    end
    target_name = get_arg(cmd, '-o')

    cmd_copy = ""
    for i in 0..cmd.length-1
      if cmd[i].end_with?(".c", ".C")
        source_name = cmd[i][0..-3]
      elsif cmd[i].end_with?(".cc")
        source_name = cmd[i][0..-4]
      elsif cmd[i].end_with?(".cpp",".cxx",".c++")
        source_name = cmd[i][0..-5]
      end
      if cmd[i] == "-fPIC"
        cmd_copy = "-fPIC"
      end
    end

    target_cov_name = source_name.gsub("/","_").gsub("-","_").gsub(".","_").gsub("+","_")

    return super if target_name =~ /conftest/

    if target_name and target_name.end_with?('.o')
      begin

        target_local_name = mangle(File.join(state.objects_directory,target_name), '.o', '.loc.o')
        target_global_name = mangle(File.join(state.objects_directory,target_name), '.o', '.glob.o')
        FileUtils.mkdir_p(File.dirname(target_global_name))
        orig_name = mangle(File.join(state.objects_directory,target_name), '.o', '.orig.bc')
        cov_name = mangle(File.join(state.objects_directory,target_name), '.o', '.cov.o')
        covbc_name = mangle(File.join(state.objects_directory,target_name), '.o', '.cov.bc')
        

        clang_args = cmd[1..-1]
        clang_args = ['-gline-tables-only',"-flto"] + clang_args
        run!(clang, *clang_args, "-o", orig_name)
        
        run!("#{state.state_path}/../coverage.sh",target_cov_name,target_global_name, "#{state.state_path}"+"/", cmd_copy)

        # FileUtils.cp("./"+target_name, orig_name)
        run!(find_opt(), '-load', 'SRPass.so', '-dcc', "-o", cov_name, orig_name)
        run!(find_opt(), '-load', 'SRPass.so', '-dcc', "-o", covbc_name, orig_name)
        opt_level = get_optlevel_for_llc(clang_args)
        run!(find_llc(), opt_level, '-filetype=obj', '-relocation-model=pic', '-o', target_local_name, cov_name)

        run!("ld","-r",target_global_name, target_local_name,"-o",target_name_o)
        
      rescue RunExternalCommandError
        # Nothing to do...
      end
    elsif target_name and target_name.end_with?('.lo')
      begin

        target_local_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.loc.o')
        target_global_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.glob.o')
        FileUtils.mkdir_p(File.dirname(target_global_name))
        orig_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.orig.bc')
        cov_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.cov.o')
        covbc_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.cov.bc')
        

        clang_args = cmd[1..-1]
        clang_args = ['-gline-tables-only',"-flto"] + clang_args
        run!(clang, *clang_args, "-o", orig_name)
        
        run!("#{state.state_path}/../coverage.sh",target_cov_name,target_global_name, "#{state.state_path}"+"/", cmd_copy)

        # FileUtils.cp("./"+target_name, orig_name)
        run!(find_opt(), '-load', 'SRPass.so', '-dcc', "-o", cov_name, orig_name)
        run!(find_opt(), '-load', 'SRPass.so', '-dcc', "-o", covbc_name, orig_name)
        opt_level = get_optlevel_for_llc(clang_args)
        run!(find_llc(), opt_level, '-filetype=obj', '-relocation-model=pic', '-o', target_local_name, cov_name)

        run!("ld","-r",target_global_name, target_local_name,"-o",target_name_o)
        
      rescue RunExternalCommandError
        # Nothing to do...
      end

    end
  end

  def do_link(cmd)
    linker_args = cmd[1..-1]
    super([cmd[0]] + linker_args)
  end
end





# Compiler for SanRazor's fourth stage. Compiles an optimized program.
class SROptimizingCompiler < BaseCompiler
  def do_compile(cmd)
    clang = find_clang()
    
    target_name = get_arg(cmd, '-o')
    for i in 0..cmd.length-1
      if cmd[i].end_with?(".c", ".C")
        source_name = cmd[i][0..-3]
      elsif cmd[i].end_with?(".cc")
        source_name = cmd[i][0..-4]
      elsif cmd[i].end_with?(".cpp",".cxx",".c++")
        source_name = cmd[i][0..-5]
      end

      if cmd[i].start_with?("-fsanitize=address")
        san_type = "asan"
      elsif cmd[i].start_with?("-fsanitize=undefined")
        san_type = "ubsan"
      end

    end
    
    # @san_level = "L0"
    threshold = state.use_asap
    target_cov_name = source_name.gsub("/","_").gsub("-","_").gsub(".","_").gsub("+","_")
    if target_name and target_name.end_with?('.o')

      orig_name = mangle(File.join(state.objects_directory,target_name), '.o', '.orig.bc')
      sr_name = mangle(File.join(state.objects_directory,target_name), '.o', '.sr.o')
      srbc_name = mangle(File.join(state.objects_directory,target_name), '.o', '.sr.bc')
      opt_name = mangle(File.join(state.objects_directory,target_name), '.o', '.opt.o')
      scov_name = File.join(state.state_path,"/"+target_cov_name+"_SC.txt")
      ucov_name = File.join(state.state_path,"/"+target_cov_name+"_UC.txt")
      log_name = File.join(state.state_path,"/check.txt")
      target_bc_name = mangle(File.join(state.objects_directory,target_name), '.o', '.bc')
      # return super unless [orig_name, target_global_name].all? { |f| File.file?(f) }
      clang_args = cmd[1..-1]

      puts "***********************"
      puts san_type

      run!(find_opt(), '-load', 'SRPass.so', "-sr-analysis", "-sropt-level=#{state.san_level}", "-scov=#{scov_name}", "-ucov=#{ucov_name}", "-log=#{log_name}", "-use-asap=#{threshold}", "-o", sr_name, orig_name)
      run!(find_opt(), '-load', 'SRPass.so', "-sr-analysis", "-sropt-level=#{state.san_level}", "-scov=#{scov_name}", "-ucov=#{ucov_name}", "-log=#{log_name}", "-use-asap=#{threshold}", "-o", srbc_name, orig_name)

      opt_level = get_optlevel_for_llc(clang_args)
      run!(find_opt(), opt_level, '-o', opt_name, sr_name)
      run!(find_opt(), '-load', 'SRPass.so', '-scclean', '-o', opt_name, opt_name)
      run!(find_opt(), opt_level, '-o', opt_name, opt_name)
      run!(find_llc(), opt_level, '-filetype=obj', '-relocation-model=pic', '-o', target_name, opt_name)
      run!(find_llc(), opt_level, '-filetype=obj', '-relocation-model=pic', '-o', target_bc_name, opt_name)
      
    elsif target_name and target_name.end_with?('.lo')
      orig_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.orig.bc')
      sr_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.sr.o')
      srbc_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.sr.bc')
      opt_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.opt.o')
      scov_name = File.join(state.state_path,"/"+target_cov_name+"_SC.txt")
      ucov_name = File.join(state.state_path,"/"+target_cov_name+"_UC.txt")
      log_name = File.join(state.state_path,"/check.txt")
      target_bc_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.bc')
      # return super unless [orig_name, target_global_name].all? { |f| File.file?(f) }
      clang_args = cmd[1..-1]

      puts "***********************"
      puts san_type

      run!(find_opt(), '-load', 'SRPass.so', "-sr-analysis", "-sropt-level=#{state.san_level}", "-scov=#{scov_name}", "-ucov=#{ucov_name}", "-log=#{log_name}", "-use-asap=#{threshold}", "-o", sr_name, orig_name)
      run!(find_opt(), '-load', 'SRPass.so', "-sr-analysis", "-sropt-level=#{state.san_level}", "-scov=#{scov_name}", "-ucov=#{ucov_name}", "-log=#{log_name}", "-use-asap=#{threshold}", "-o", srbc_name, orig_name)

      
      opt_level = get_optlevel_for_llc(clang_args)
      run!(find_opt(), opt_level, '-o', opt_name, sr_name)
      run!(find_opt(), '-load', 'SRPass.so', '-scclean', '-o', opt_name, opt_name)
      run!(find_opt(), opt_level, '-o', opt_name, opt_name)
      run!(find_llc(), opt_level, '-filetype=obj', '-relocation-model=pic', '-o', target_name, opt_name)
      run!(find_llc(), opt_level, '-filetype=obj', '-relocation-model=pic', '-o', target_bc_name, opt_name)

    end

  end
end

# Compiler for SR's fourth stage. Compiles an optimized program.
class SRSanCompiler < BaseCompiler
  def do_compile(cmd)
    clang = find_clang()
    
    target_name = get_arg(cmd, '-o')
    for i in 0..cmd.length-1
      if cmd[i].end_with?(".c", ".C")
        source_name = cmd[i][0..-3]
      elsif cmd[i].end_with?(".cc")
        source_name = cmd[i][0..-4]
      elsif cmd[i].end_with?(".cpp",".cxx",".c++")
        source_name = cmd[i][0..-5]
      end

    end
    target_cov_name = source_name.gsub("/","_").gsub("-","_")
    if target_name and target_name.end_with?('.o')
      orig_name = mangle(File.join(state.objects_directory,target_name), '.o', '.orig.bc')
      opt_name = mangle(File.join(state.objects_directory,target_name), '.o', '.opt.o')
    elsif target_name and target_name.end_with?('.lo')
      orig_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.orig.bc')
      opt_name = mangle(File.join(state.objects_directory,target_name), '.lo', '.opt.o')
    end

    clang_args = cmd[1..-1]

    opt_level = get_optlevel_for_llc(clang_args)
    run!(find_opt(), opt_level, '-o', opt_name, orig_name)
    run!(find_llc(), opt_level, '-filetype=obj', '-relocation-model=pic', '-o', target_name, opt_name)

  end
end


# Some makefiles compile and link with a single command. We need to handle this
# specially and convert it into multiple commands.
def handle_compile_and_link(argv)
  source_files = argv.select { |f| f =~ /\.(?:c|cc|C|cxx|cpp)$/ }
  is_compile = get_arg(argv, '-c', :first)
  output_file = get_arg(argv, '-o')
  return false if source_files.empty? or is_compile or not output_file

  # OK, this is a combined compile-and-link.
  # Replace it with multiple commands, where each command compiles a single source file.
  non_source_opts = argv.select { |a| not source_files.include?(a) }
  source_files.each do |f|
    current_args = non_source_opts.collect { |a| if a == output_file then "#{f}.o" else a end }
    current_args += ['-c', f]
    main(current_args)
  end
  # Add a link command
  link_args = argv.collect { |a| if source_files.include?(a) then "#{a}.o" else a end }
  main(link_args)

  return true
end

# Some makefiles compile without specifying -o, relying on compilers to choose
# the name of the object file.
def handle_missing_output_name(argv)
  source_files = argv.select { |f| f =~ /\.(?:c|cc|C|cxx|cpp)$/ }
  is_compile = get_arg(argv, '-c', :first)
  output_file = get_arg(argv, '-o')
  return false if source_files.size != 1 or not is_compile or output_file

  # Add the output name manually. The default compiler behavior is to replace
  # the extension with .o, and place the file in the current working directory.
  output_file = source_files[0].sub(/\.(?:c|cc|C|cxx|cpp)$/, '.o')
  output_file = File.basename(output_file)
  main(argv + ['-o', output_file])
  return true
end

# Some build systems (libtool, I'm looking at you) use -MF and related options.
# We create empty dependency files to make them happy. Note that this is a
# hack... for example, it doesn't handle the case when -M is given without -MF,
# and it will break dependency tracking.
def handle_mf_option(argv)
  is_compile = get_arg(argv, '-c', :first)
  dependency_file = get_arg(argv, '-MF')

  if is_compile and dependency_file
    IO.write(dependency_file, "# Stub dependency file created by SR-clang")
  end

  return false  # continue the compilation anyway
end

def main(argv)
  command = get_arg(argv, /^-SR-[a-z0-9-]+$/, :first)
  if command.nil?
    # We are being run like a regular compilation tool.

    # First, handle a few compiler/makefile quirks
    return if handle_compile_and_link(argv)
    return if handle_missing_output_name(argv)
    return if handle_mf_option(argv)

    # Figure out the right
    # compilation stage, and run the corresponding command.
    state = SRState.new
    compiler = state.create_compiler
    compiler.exec([$0] + argv)
  elsif command == '-SR-init'
    SRState.initialize_state(:L0)
  elsif command == '-SR-opt'
    san_level = get_arg(argv, '-san-level=')
    use_asap = get_arg(argv, "-use-asap=")
    puts san_level
    puts san_level
    puts use_asap
    state = SRState.new
    state.san_level = san_level
    state.use_asap = use_asap
    # state.transition(:initial, :optimize) do
    #   puts "Will build reduced version on next rebuild; please run:"
    #   puts "make clean && make"
    # end
    # state = SRState.new
    state.current_state = :optimize
  elsif command == "-SR-san"
    state = SRState.new
    state.current_state = :sanitizer
  else
    raise "unknown command: #{command}"
  end
end
main(ARGV)
