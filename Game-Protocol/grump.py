#!/usr/bin/env python3
from __future__ import print_function
import sys
import os

# Get correct path to files, based on platform
import platform
host = platform.node().split('.')[0]
sys.path[0:0] = ['/home/students/s4436755/public/libmarks-new']
TEST_LOCATION='/local/courses/csse2310/resources/tests/a3ptests/tests'

import marks

COMPILE = 'make'

# Useful constants
SIGHUP = 1

# Programs outside of tests
SERV = '/local/courses/csse2310/resources/assignment3/demo_rpsserver'
RPS = '/local/courses/csse2310/resources/assignment3/demo_rpsclient'
RPS_TIE = '/local/courses/csse2310/resources/assignment3/demo_rpsclient_cheat_tie'
RPS_LOSE = '/local/courses/csse2310/resources/assignment3/demo_rpsclient_cheat_lose'
RPS_WIN = '/local/courses/csse2310/resources/assignment3/demo_rpsclient_cheat_win'

class Assignment3(marks.TestCase):
    timeout = 10
    @classmethod
    def setup_class(cls):
        # Store original location
        options = getattr(cls, '__marks_options__', {})

        cls._rpsserver = os.path.join(options['working_dir'], './rpsserver')
        cls._rpsclient = os.path.join(options['working_dir'], './rpsclient')

        cls._compile_warnings = 0
        cls._compile_errors = 0

        # Create symlink to tests in working dir
        os.chdir(options['working_dir'])
        try:
            os.symlink(TEST_LOCATION, 'tests')
        except OSError:
            pass

        os.chdir(options['temp_dir'])

        # Modify test environment when running tests (excl. explain mode).
        if not options.get('explain', False):
            # Compile program
            os.chdir(options['working_dir'])
            p = marks.Process(COMPILE.split())
            os.chdir(options['temp_dir'])

            # Count warnings and errors
            warnings = 0
            errors = 0
            while True:
                line = p.readline_stderr()
                if line == '':
                    break
                if 'warning:' in line:
                    warnings += 1
                if 'error:' in line:
                    errors += 1
                print(line, end='')
            
            # Do not run tests if compilation failed.
            assert p.assert_exit_status(0)
            
            # Create symlink to tests within temp folder
            try:
                os.symlink(TEST_LOCATION, 'tests')
            except OSError:
                pass
    
    def get_netdog_port(self, nd):
        try:
            if self.option('explain'):
                self.explain("netdog prints its port to stderr."\
                    "Copy this when starting your rpsclients.")
                return "<netdog_port>"
            else:
                return nd.readline_stdout().strip()
        except e:
            print(e)

    @marks.marks('dummy', category_marks=1)
    def test_servUsage1(self):
        _rpsserver = self.process([self._rpsserver] + ['some', 'args', 'here'])
        _rpsserver.finish_input()
        self.assert_stdout_matches_file(_rpsserver, 'tests/common/empty')
        self.assert_stderr_matches_file(_rpsserver, 'tests/server/usage.err')
        self.assert_exit_status(_rpsserver, 1)

    @marks.marks('dummy', category_marks=1)
    def test_clientUsage1(self):
        _rpsclient = self.process([self._rpsclient] + ['""'])
        _rpsclient.finish_input()
        self.assert_stdout_matches_file(_rpsclient, 'tests/common/empty')
        self.assert_stderr_matches_file(_rpsclient, 'tests/client/usage.err')
        self.assert_exit_status(_rpsclient, 1)

    @marks.marks('dummy', category_marks=1)
    def test_clientUsage2(self):
        _rpsclient = self.process([self._rpsclient] + ['My N4me', '1', '2310'])
        _rpsclient.finish_input()
        self.assert_stdout_matches_file(_rpsclient, 'tests/common/empty')
        self.assert_stderr_matches_file(_rpsclient, 'tests/client/badname.err')
        self.assert_exit_status(_rpsclient, 2)

    @marks.marks('dummy', category_marks=1)
    def test_clientUsage3(self):
        _rpsclient = self.process([self._rpsclient] + ['2310-client', '1', '2310'])
        _rpsclient.finish_input()
        self.assert_stdout_matches_file(_rpsclient, 'tests/common/empty')
        self.assert_stderr_matches_file(_rpsclient, 'tests/client/badname.err')
        self.assert_exit_status(_rpsclient, 2)

    @marks.marks('dummy', category_marks=1)
    def test_clientUsage4(self):
        _rpsclient = self.process([self._rpsclient] + ['2310client', '0', '2310'])
        _rpsclient.finish_input()
        self.assert_stdout_matches_file(_rpsclient, 'tests/common/empty')
        self.assert_stderr_matches_file(_rpsclient, 'tests/client/badcount.err')
        self.assert_exit_status(_rpsclient, 3)

    @marks.marks('dummy', category_marks=1)
    def test_clientUsage5(self):
        _rpsclient = self.process([self._rpsclient] + ['2310client', '1', '23lO'])
        _rpsclient.finish_input()
        self.assert_stdout_matches_file(_rpsclient, 'tests/common/empty')
        self.assert_stderr_matches_file(_rpsclient, 'tests/client/badport.err')
        self.assert_exit_status(_rpsclient, 4)

    @marks.marks('dummy', category_marks=1)
    def test_clientUsage6(self):
        _rpsclient = self.process([self._rpsclient] + ['2310client', '1', '100000'])
        _rpsclient.finish_input()
        self.assert_stdout_matches_file(_rpsclient, 'tests/common/empty')
        self.assert_stderr_matches_file(_rpsclient, 'tests/client/badport.err')
        self.assert_exit_status(_rpsclient, 4)

    @marks.marks('dummy', category_marks=1)
    def test_clientMR1(self):
        # Start up netdog to act as a server
        nd1 = self.process(['tests/fuzzify.sh', 'netdog'])
        if self.option('explain'):
            port = "<netdog_port>"
        else:
            port = nd1.readline_stdout().strip()
        # Make the client connect to it and send the request
        _rpsclient = self.process([self._rpsclient] + ['2310client', '1', port])
        self.delay(1)
        self.assert_stdout_matches_file(nd1, 'tests/client/matchreq.1.out')
    
    @marks.marks('dummy', category_marks=1)
    def test_full1(self):
        serv = self.process([self._rpsserver])
        
        if self.option('explain'):
            port = "<serv_port>"
        else:
            port = serv.readline_stdout().strip()

        c1 = self.process([self._rpsclient, 'bob', '1', port])
        c2 = self.process([self._rpsclient, 'steve', '1', port])
        self.delay(2)
        serv.send_signal(SIGHUP)
        self.delay(1)

        self.assert_stdout_matches_file(c1, 'tests/client/full.1.1.out')
        self.assert_stdout_matches_file(c2, 'tests/client/full.1.2.out')
        self.assert_stdout_matches_file(serv, 'tests/server/full.1.1.out')
    
    @marks.marks('dummy', category_marks=1)
    def test_full2(self):
        serv = self.process([self._rpsserver])
        
        if self.option('explain'):
            port = "<serv_port>"
        else:
            port = serv.readline_stdout().strip()

        c1 = self.process([RPS_WIN, 'fred', '1', port])
        c2 = self.process([self._rpsclient, 'wilma', '1', port])
        self.delay(2)
        serv.send_signal(SIGHUP)
        self.delay(1)

        self.assert_stdout_matches_file(c2, 'tests/client/full.2.2.out')
        self.assert_stdout_matches_file(serv, 'tests/server/full.2.1.out')
     
    @marks.marks('dummy', category_marks=1)
    def test_full3(self):
        serv = self.process([self._rpsserver])
        
        if self.option('explain'):
            port = "<serv_port>"
        else:
            port = serv.readline_stdout().strip()

        c1 = self.process([RPS_LOSE, 'clyde', '1', port])
        c2 = self.process([self._rpsclient, 'bonnie', '1', port])
        self.delay(2)
        serv.send_signal(SIGHUP)
        self.delay(1)

        self.assert_stdout_matches_file(c2, 'tests/client/full.3.2.out')
        self.assert_stdout_matches_file(serv, 'tests/server/full.3.1.out')
     
    @marks.marks('dummy', category_marks=1)
    def test_full4(self):
        serv = self.process([self._rpsserver])
        
        if self.option('explain'):
            port = "<serv_port>"
        else:
            port = serv.readline_stdout().strip()

        c1 = self.process([RPS_TIE, 'kim', '1', port])
        c2 = self.process([self._rpsclient, 'kath', '1', port])
        self.delay(2)
        serv.send_signal(SIGHUP)
        self.delay(1)

        self.assert_stdout_matches_file(c2, 'tests/client/full.4.2.out')
        self.assert_stdout_matches_file(serv, 'tests/server/full.4.1.out')
    
    @marks.marks('dummy', category_marks=1)
    def test_disco1(self):
        serv = self.process([SERV])
        
        if self.option('explain'):
            port = "<serv_port>"
        else:
            port = serv.readline_stdout().strip()

        c1 = self.process([self._rpsclient, 'bob', '1', port])
        self.delay(1)
        serv.kill()

        self.assert_stdout_matches_file(c1, 'tests/common/empty')
        self.assert_stderr_matches_file(c1, 'tests/client/badport.err')
    
    def test_multi1(self):
        serv = self.process([self._rpsserver])
        
        if self.option('explain'):
            port = "<serv_port>"
        else:
            port = serv.readline_stdout().strip()

        c1 = self.process([self._rpsclient, 'kim', '15', port])
        c2 = self.process([self._rpsclient, 'bob', '1', port])
        self.delay(1)
        c3 = self.process([self._rpsclient, 'kath', '14', port])
        self.delay(2)
        serv.send_signal(SIGHUP)
        self.delay(1)

        self.assert_stdout_matches_file(c1, 'tests/client/multi.1.1.out')
        self.assert_stdout_matches_file(c2, 'tests/client/multi.1.2.out')
        self.assert_stdout_matches_file(c3, 'tests/client/multi.1.3.out')
        self.assert_stdout_matches_file(serv, 'tests/server/multi.1.1.out')
    
if __name__=='__main__':
    marks.set_ld_preload('/local/courses/csse2310/lib/protect/protect.so')
    marks.main()

