#! /usr/bin/ruby

# TODOLIST:
# - refactor into OO (stub can become a class)
# - think about use of tags like @failure_msg
# - there's code to be factored out from make_bin and make_asm

# the convention for dirnames is that they don't end with '/'
if ENV['PATH'] !~ /archc/
  ENV['PATH'] = '/l/archc/compilers/bin:/l/archc/bin:' << ENV['PATH']
end
ALTIVEC_HOME = ENV['ALTIVEC_HOME'] || '/home/helder/work/altivec'
ISA_CPP = ALTIVEC_HOME+'/model/powerpc_isa.cpp'
TEST_PREFIX = 'test'
TESTS_HOME = ALTIVEC_HOME+'/tests'
TESTS_CODE = TESTS_HOME+'/code'
TESTS_BIN = TESTS_HOME+'/bin'
TESTS_LOG = TESTS_HOME+'/log'
FAIL_MSG_TAG = '@failure_msg'


# returns the names of the test files
def test_files
  Dir.open(TESTS_CODE) do |dir|
    dir.entries.grep(/^#{TEST_PREFIX}/)
  end
end

# returns a Hash with filename => output
def make_bin(test_files)
  Dir.mkdir(TESTS_BIN) unless File.exists?(TESTS_BIN)
  log = {}
  # in the remote case of porting to windows, this should use Open3 instead of '2>&1'
  test_files.each do |f|
    output = `powerpc-elf-gcc -O0 --specs=archc #{TESTS_CODE+'/'+f} -o #{TESTS_BIN+'/'+f[0..-3]+'.o'} 2>&1`
    log[f] = output == '' ? "No error." : output
  end
  return log
end

# returns a Hash with filename => output
def make_asm(test_files)
  Dir.mkdir(TESTS_BIN) unless File.exists?(TESTS_BIN)
  log = {}
  # in the remote case of porting to windows, this should use Open3 instead of '2>&1'
  test_files.each do |f|
    output = `powerpc-elf-gcc -O0 --specs=archc -S #{TESTS_CODE+'/'+f} -o #{TESTS_BIN+'/'+f[0..-3]+'.s'} 2>&1`
    log[f] = output == '' ? "No error." : output
  end
  return log
end

def log_entry(test_file, text)
<<-end
#### TEST FILE: #{test_file} ####
#{text}

   end

end

def write_log(filename,log)
  Dir.mkdir(TESTS_LOG) unless File.exists?(TESTS_LOG)
  file = File.new(TESTS_LOG+'/'+filename,'a') 
  log.each do |instr, text|
    file << log_entry(instr,text)
  end
  file.close
end

def make
  tests = test_files()
  log_bin = make_bin(tests)
  log_asm = make_asm(tests)
  write_log('make_bin.log',log_bin)
  write_log('make_asm.log',log_asm)
end

def create_stubs 
  cpp_lines = File.new(ISA_CPP,'r').entries
  instrs = instr_names(cpp_lines)
  instrs.each {|i| write_stub(stub_filename(i), stub(i))}
end

def write_stub(filename, stub)
  Dir.mkdir(TESTS_CODE) unless File.exists?(TESTS_CODE)
  file = File.new(TESTS_CODE+"/#{filename}",'w')
  file << stub
  file.close
end

def stub_filename(instr_name)
  # TODO fazer o numero do arquivo ser atualizado, para que
  # se possa criar novos stubs para uma mesma instrução com:
  # ./test create-stub instrucao
 TEST_PREFIX + "_" + instr_name + '00.c' 
end

def instr_names(cpp_lines)
  # pega as linhas a partir de onde tiver comentário com 'altivec'
  start_line = 0
  cpp_lines.each_with_index do |l,i|
    # FIXME: this line is a major bottleneck. speed this up.
    start_line = i if cpp_lines.grep(/altivec/i)[0] == l 
  end
  # ac_behavior( ) 
  decls = cpp_lines[start_line..-1].grep(/ac_behavior/) 
  # declaracoes q tem upcase sao de tipos de instrucao, nao de instrucao
  instrs = decls.select {|l| l == l.downcase} 
  # 'void ac_behavior( lvebx ) {\n' => queremos soh o lvebx
  instrs = instrs.map {|l| l =~ /\((.*)\)/ ; $1.strip}
  return instrs
end

def stub(instr_name)
  <<-end
/* 
 * stub for instruction: #{instr_name}
 * remember each test file should perform *one and only one*
 * test, and return 0 on success or 1 on failure.
 * copy this stub for aditional tests on the same instruction.
 * 
 * ALL TEST FILES MUST HAVE NAME BEGINNING WITH '#{TEST_PREFIX}'
 * IN ORDER TO BE INCLUDED IN THE AUTOMATIC BATCH TESTING
 */ 

/* one-line #{FAIL_MSG_TAG}: 
 */
int main() {



}
  end
end

create_stubs()
make()


