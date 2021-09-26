#!/usr/bin/env python
from __future__ import print_function
import sys
import os

# Get correct path to files, based on platform
import platform
host = platform.node().split('.')[0]
sys.path[0:0] = ['/local/courses/csse2310/lib']
TEST_LOCATION='/local/courses/csse2310/resources/tests/a1mtests/tests'

import marks

COMPILE = 'make'


class Assignment1(marks.TestCase):
    timeout = 12
    @classmethod
    def setup_class(cls):
        # Store original location
        options = getattr(cls, '__marks_options__', {})

        cls.naval = os.path.join(options['working_dir'], './naval')

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

    def setup(self):
        options = getattr(self, '__marks_options__', {})
        if not options.get('explain', False):
            import shutil
            try:
                shutil.copyfile(TEST_LOCATION + "/standard.rules",
                        options['temp_dir'] + "/standard.rules")
            except OSError:
                pass

    def tear_down(self):
        options = getattr(self, '__marks_options__', {})
        if not options.get('explain', False):
            try:
                os.remove(options['temp_dir'] + "/standard.rules")
            except OSError:
                pass

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage1(self):
        naval = self.process([self.naval] + [])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badparams.err')
        self.assert_exit_status(naval, 10)

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage2(self):
        naval = self.process([self.naval] + ['standard.rules'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badparams.err')
        self.assert_exit_status(naval, 10)

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badparams.err')
        self.assert_exit_status(naval, 10)

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/1.map.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badparams.err')
        self.assert_exit_status(naval, 10)

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage5(self):
        naval = self.process([self.naval] + ['norules', 'tests/1.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/norules.err')
        self.assert_exit_status(naval, 20)

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage6(self):
        naval = self.process([self.naval] + ['standard.rules', 'nomap', 'tests/1.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/noplrmap.err')
        self.assert_exit_status(naval, 30)

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage7(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'nomap', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/nocpumap.err')
        self.assert_exit_status(naval, 31)

    @marks.marks('1-cmdline', category_marks=2)
    def test_usage8(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/1.map.cpu', 'noturns'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/nocputurns.err')
        self.assert_exit_status(naval, 40)

    @marks.marks('2-stdrules-east-hit', category_marks=2)
    def test_easthit1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/1.easthit.turns.cpu'], 'tests/1.easthit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.easthit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-hit', category_marks=2)
    def test_easthit2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/2.easthit.turns.cpu'], 'tests/2.easthit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.easthit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-hit', category_marks=2)
    def test_easthit3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/3.easthit.turns.cpu'], 'tests/3.easthit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.easthit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-hit', category_marks=2)
    def test_easthit4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/4.easthit.turns.cpu'], 'tests/4.easthit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.easthit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-miss', category_marks=2)
    def test_eastmiss1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/1.eastmiss.turns.cpu'], 'tests/1.eastmiss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.eastmiss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-miss', category_marks=2)
    def test_eastmiss2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/2.eastmiss.turns.cpu'], 'tests/2.eastmiss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.eastmiss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-miss', category_marks=2)
    def test_eastmiss3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/3.eastmiss.turns.cpu'], 'tests/3.eastmiss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.eastmiss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-miss', category_marks=2)
    def test_eastmiss4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/4.eastmiss.turns.cpu'], 'tests/4.eastmiss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.eastmiss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-sunk', category_marks=2)
    def test_eastsunk1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/1.eastmiss.turns.cpu'], 'tests/1.eastsunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.eastsunk.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-sunk', category_marks=2)
    def test_eastsunk2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/2.eastmiss.turns.cpu'], 'tests/2.eastsunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.eastsunk.out')
        self.assert_stderr_matches_file(naval, 'tests/giveup.err')
        self.assert_exit_status(naval, 140)

    @marks.marks('2-stdrules-east-sunk', category_marks=2)
    def test_eastsunk3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/3.eastmiss.turns.cpu'], 'tests/3.eastsunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.eastsunk.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-sunk', category_marks=2)
    def test_eastsunk4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/2.map.cpu', 'tests/4.eastsunk.turns.cpu'], 'tests/4.eastsunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.eastsunk.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-bad-guess', category_marks=1)
    def test_badguess1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'], 'tests/1.badguess.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-bad-guess', category_marks=1)
    def test_badguess2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'], 'tests/2.badguess.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-bad-guess', category_marks=1)
    def test_badguess3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'], 'tests/3.badguess.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-bad-guess', category_marks=1)
    def test_badguess4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/1.map.cpu', 'tests/4.badguess.turns.cpu'], 'tests/1.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-wholegame', category_marks=7)
    def test_eastfull1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.eastfull.map.plr', 'tests/1.eastfull.map.cpu', 'tests/1.eastfull.turns.cpu'], 'tests/1.eastfull.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.eastfull.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('2-stdrules-east-wholegame', category_marks=7)
    def test_eastfull2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.eastfull.map.plr', 'tests/2.eastfull.map.cpu', 'tests/2.eastfull.turns.cpu'], 'tests/2.eastfull.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.eastfull.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('2-stdrules-east-wholegame', category_marks=7)
    def test_eastfull3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/3.eastfull.map.plr', 'tests/3.eastfull.map.cpu', 'tests/3.eastfull.turns.cpu'], 'tests/3.eastfull.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.eastfull.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('2-stdrules-east-wholegame', category_marks=7)
    def test_eastfull4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/4.eastfull.map.plr', 'tests/4.eastfull.map.cpu', 'tests/4.eastfull.turns.cpu'], 'tests/4.eastfull.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.eastfull.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('2-stdrules-east-wholegame', category_marks=7)
    def test_eastfull5(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/5.eastfull.map.plr', 'tests/5.eastfull.map.cpu', 'tests/5.eastfull.turns.cpu'], 'tests/5.eastfull.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/5.eastfull.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('2-stdrules-east-wholegame', category_marks=7)
    def test_eastfull6(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/6.eastfull.map.plr', 'tests/6.eastfull.map.cpu', 'tests/6.eastfull.turns.cpu'], 'tests/6.eastfull.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/6.eastfull.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('2-stdrules-east-wholegame', category_marks=7)
    def test_eastfull7(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/7.eastfull.map.plr', 'tests/7.eastfull.map.cpu', 'tests/7.eastfull.turns.cpu'], 'tests/7.eastfull.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/7.eastfull.out')
        self.assert_stderr_matches_file(naval, 'tests/giveup.err')
        self.assert_exit_status(naval, 140)

    @marks.marks('2-stdrules-east-inv-map', category_marks=2)
    def test_badmap1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.badmap.map.plr', 'tests/2.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/plr.overlap.err')
        self.assert_exit_status(naval, 60)

    @marks.marks('2-stdrules-east-inv-map', category_marks=2)
    def test_badmap2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.badmap.map.plr', 'tests/2.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/plrbounds.err')
        self.assert_exit_status(naval, 80)

    @marks.marks('2-stdrules-east-inv-map', category_marks=2)
    def test_badmap3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/3.badmap.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/cpu.overlap.err')
        self.assert_exit_status(naval, 70)

    @marks.marks('2-stdrules-east-inv-map', category_marks=2)
    def test_badmap4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/4.badmap.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/cpubounds.err')
        self.assert_exit_status(naval, 90)

    @marks.marks('2-stdrules-east-inv-map', category_marks=2)
    def test_badmap5(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/5.badmap.map.plr', 'tests/2.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badplrmap.err')
        self.assert_exit_status(naval, 100)

    @marks.marks('2-stdrules-east-inv-map', category_marks=2)
    def test_badmap6(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.map.plr', 'tests/6.badmap.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badcpumap.err')
        self.assert_exit_status(naval, 110)

    @marks.marks('3-stdrules-hit', category_marks=2)
    def test_hit1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.hit.map.plr', 'tests/1.hit.map.cpu', 'tests/1.hit.turns.cpu'], 'tests/1.hit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.hit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-hit', category_marks=2)
    def test_hit2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.hit.map.plr', 'tests/2.hit.map.cpu', 'tests/2.hit.turns.cpu'], 'tests/2.hit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.hit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-hit', category_marks=2)
    def test_hit3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/3.hit.map.plr', 'tests/3.hit.map.cpu', 'tests/3.hit.turns.cpu'], 'tests/3.hit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.hit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-hit', category_marks=2)
    def test_hit4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/4.hit.map.plr', 'tests/4.hit.map.cpu', 'tests/4.hit.turns.cpu'], 'tests/4.hit.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.hit.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-miss', category_marks=2)
    def test_miss1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.miss.map.plr', 'tests/1.miss.map.cpu', 'tests/1.miss.turns.cpu'], 'tests/1.miss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.miss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-miss', category_marks=2)
    def test_miss2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.miss.map.plr', 'tests/2.miss.map.cpu', 'tests/2.miss.turns.cpu'], 'tests/2.miss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.miss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-miss', category_marks=2)
    def test_miss3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/3.miss.map.plr', 'tests/3.miss.map.cpu', 'tests/3.miss.turns.cpu'], 'tests/3.miss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.miss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-miss', category_marks=2)
    def test_miss4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/4.miss.map.plr', 'tests/4.miss.map.cpu', 'tests/4.miss.turns.cpu'], 'tests/4.miss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.miss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-miss', category_marks=2)
    def test_miss5(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/5.miss.map.plr', 'tests/5.miss.map.cpu', 'tests/5.miss.turns.cpu'], 'tests/5.miss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/5.miss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-miss', category_marks=2)
    def test_miss6(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/6.miss.map.plr', 'tests/6.miss.map.cpu', 'tests/6.miss.turns.cpu'], 'tests/6.miss.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/6.miss.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-sunk', category_marks=2)
    def test_sunk1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.sunk.map.plr', 'tests/1.sunk.map.cpu', 'tests/1.sunk.turns.cpu'], 'tests/1.sunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.sunk.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-sunk', category_marks=2)
    def test_sunk2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.sunk.map.plr', 'tests/2.sunk.map.cpu', 'tests/2.sunk.turns.cpu'], 'tests/2.sunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.sunk.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-sunk', category_marks=2)
    def test_sunk3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/3.sunk.map.plr', 'tests/3.sunk.map.cpu', 'tests/3.sunk.turns.cpu'], 'tests/3.sunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.sunk.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-sunk', category_marks=2)
    def test_sunk4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/4.sunk.map.plr', 'tests/4.sunk.map.cpu', 'tests/4.sunk.turns.cpu'], 'tests/4.sunk.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.sunk.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-bad-guess', category_marks=1)
    def test_badguess5(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/5.badguess.map.plr', 'tests/5.badguess.map.cpu', 'tests/5.badguess.turns.cpu'], 'tests/5.badguess.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/5.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-bad-guess', category_marks=1)
    def test_badguess6(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/6.badguess.map.plr', 'tests/6.badguess.map.cpu', 'tests/6.badguess.turns.cpu'], 'tests/6.badguess.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/6.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/giveup.err')
        self.assert_exit_status(naval, 140)

    @marks.marks('3-stdrules-bad-guess', category_marks=1)
    def test_badguess7(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/7.badguess.map.plr', 'tests/7.badguess.map.cpu', 'tests/7.badguess.turns.cpu'], 'tests/7.badguess.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/7.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-bad-guess', category_marks=1)
    def test_badguess8(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/8.badguess.map.plr', 'tests/8.badguess.map.cpu', 'tests/8.badguess.turns.cpu'], 'tests/8.badguess.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/8.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full1(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.full.map.plr', 'tests/1.full.map.cpu', 'tests/1.full.turns.cpu'], 'tests/1.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/1.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full2(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/2.full.map.plr', 'tests/2.full.map.cpu', 'tests/2.full.turns.cpu'], 'tests/2.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/2.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full3(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/3.full.map.plr', 'tests/3.full.map.cpu', 'tests/3.full.turns.cpu'], 'tests/3.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/3.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full4(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/4.full.map.plr', 'tests/4.full.map.cpu', 'tests/4.full.turns.cpu'], 'tests/4.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/4.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full5(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/5.full.map.plr', 'tests/5.full.map.cpu', 'tests/5.full.turns.cpu'], 'tests/5.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/5.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full6(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/6.full.map.plr', 'tests/6.full.map.cpu', 'tests/6.full.turns.cpu'], 'tests/6.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/6.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full7(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/7.full.map.plr', 'tests/7.full.map.cpu', 'tests/7.full.turns.cpu'], 'tests/7.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/7.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full8(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/8.full.map.plr', 'tests/8.full.map.cpu', 'tests/8.full.turns.cpu'], 'tests/8.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/8.full.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('3-stdrules-wholegame', category_marks=7)
    def test_full9(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/9.full.map.plr', 'tests/9.full.map.cpu', 'tests/9.full.turns.cpu'], 'tests/9.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/9.full.out')
        self.assert_stderr_matches_file(naval, 'tests/giveup.err')
        self.assert_exit_status(naval, 140)

    @marks.marks('3-stdrules-inv-map', category_marks=2)
    def test_badmap7(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/7.badmap.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/plr.overlap.err')
        self.assert_exit_status(naval, 60)

    @marks.marks('3-stdrules-inv-map', category_marks=2)
    def test_badmap8(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/8.badmap.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/plrbounds.err')
        self.assert_exit_status(naval, 80)

    @marks.marks('3-stdrules-inv-map', category_marks=2)
    def test_badmap9(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/9.badmap.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/plrbounds.err')
        self.assert_exit_status(naval, 80)

    @marks.marks('3-stdrules-inv-map', category_marks=2)
    def test_badmap10(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/10.badmap.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/cpu.overlap.err')
        self.assert_exit_status(naval, 70)

    @marks.marks('3-stdrules-inv-map', category_marks=2)
    def test_badmap11(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/11.badmap.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/cpubounds.err')
        self.assert_exit_status(naval, 90)

    @marks.marks('3-stdrules-inv-map', category_marks=2)
    def test_badmap12(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/1.map.plr', 'tests/12.badmap.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/cpubounds.err')
        self.assert_exit_status(naval, 90)

    @marks.marks('3-stdrules-inv-map', category_marks=2)
    def test_badmap13(self):
        naval = self.process([self.naval] + ['standard.rules', 'tests/13.badmap.map.plr', 'tests/1.map.cpu', 'tests/1.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badplrmap.err')
        self.assert_exit_status(naval, 100)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap14(self):
        naval = self.process([self.naval] + ['tests/14.badmap.rules', 'tests/14.badmap.map.plr', 'tests/14.badmap.map.cpu', 'tests/14.badmap.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/plr.overlap.err')
        self.assert_exit_status(naval, 60)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap15(self):
        naval = self.process([self.naval] + ['tests/15.badmap.rules', 'tests/15.badmap.map.plr', 'tests/15.badmap.map.cpu', 'tests/15.badmap.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/plrbounds.err')
        self.assert_exit_status(naval, 80)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap16(self):
        naval = self.process([self.naval] + ['tests/16.badmap.rules', 'tests/16.badmap.map.plr', 'tests/16.badmap.map.cpu', 'tests/16.badmap.turns.cpu'], 'tests/16.badmap.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/16.badmap.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap17(self):
        naval = self.process([self.naval] + ['tests/17.badmap.rules', 'tests/17.badmap.map.plr', 'tests/17.badmap.map.cpu', 'tests/17.badmap.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/cpu.overlap.err')
        self.assert_exit_status(naval, 70)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap18(self):
        naval = self.process([self.naval] + ['tests/18.badmap.rules', 'tests/18.badmap.map.plr', 'tests/18.badmap.map.cpu', 'tests/18.badmap.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/cpubounds.err')
        self.assert_exit_status(naval, 90)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap19(self):
        naval = self.process([self.naval] + ['tests/19.badmap.rules', 'tests/19.badmap.map.plr', 'tests/19.badmap.map.cpu', 'tests/19.badmap.turns.cpu'], 'tests/19.badmap.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/19.badmap.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap20(self):
        naval = self.process([self.naval] + ['tests/20.badmap.rules', 'tests/20.badmap.map.plr', 'tests/20.badmap.map.cpu', 'tests/20.badmap.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badplrmap.err')
        self.assert_exit_status(naval, 100)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap21(self):
        naval = self.process([self.naval] + ['tests/21.badmap.rules', 'tests/21.badmap.map.plr', 'tests/21.badmap.map.cpu', 'tests/21.badmap.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badcpumap.err')
        self.assert_exit_status(naval, 110)

    @marks.marks('4-rules-inv-map', category_marks=1)
    def test_badmap22(self):
        naval = self.process([self.naval] + ['tests/22.badmap.rules', 'tests/22.badmap.map.plr', 'tests/22.badmap.map.cpu', 'tests/22.badmap.turns.cpu'])
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/22.badguess.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules1(self):
        naval = self.process([self.naval] + ['tests/1.badrules.rules', 'tests/1.badrules.map.plr', 'tests/1.badrules.map.cpu', 'tests/1.badrules.turns.cpu'], 'tests/1.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules2(self):
        naval = self.process([self.naval] + ['tests/2.badrules.rules', 'tests/2.badrules.map.plr', 'tests/2.badrules.map.cpu', 'tests/2.badrules.turns.cpu'], 'tests/2.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules3(self):
        naval = self.process([self.naval] + ['tests/3.badrules.rules', 'tests/3.badrules.map.plr', 'tests/3.badrules.map.cpu', 'tests/3.badrules.turns.cpu'], 'tests/3.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules4(self):
        naval = self.process([self.naval] + ['tests/4.badrules.rules', 'tests/4.badrules.map.plr', 'tests/4.badrules.map.cpu', 'tests/4.badrules.turns.cpu'], 'tests/4.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules5(self):
        naval = self.process([self.naval] + ['tests/5.badrules.rules', 'tests/5.badrules.map.plr', 'tests/5.badrules.map.cpu', 'tests/5.badrules.turns.cpu'], 'tests/5.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules6(self):
        naval = self.process([self.naval] + ['tests/6.badrules.rules', 'tests/6.badrules.map.plr', 'tests/6.badrules.map.cpu', 'tests/6.badrules.turns.cpu'], 'tests/6.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules7(self):
        naval = self.process([self.naval] + ['tests/7.badrules.rules', 'tests/7.badrules.map.plr', 'tests/7.badrules.map.cpu', 'tests/7.badrules.turns.cpu'], 'tests/7.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules8(self):
        naval = self.process([self.naval] + ['tests/8.badrules.rules', 'tests/8.badrules.map.plr', 'tests/8.badrules.map.cpu', 'tests/8.badrules.turns.cpu'], 'tests/8.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-inv-rules', category_marks=1)
    def test_badrules9(self):
        naval = self.process([self.naval] + ['tests/9.badrules.rules', 'tests/9.badrules.map.plr', 'tests/9.badrules.map.cpu', 'tests/9.badrules.turns.cpu'], 'tests/9.badrules.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/empty')
        self.assert_stderr_matches_file(naval, 'tests/badrules.err')
        self.assert_exit_status(naval, 50)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full10(self):
        naval = self.process([self.naval] + ['tests/10.full.rules', 'tests/10.full.map.plr', 'tests/10.full.map.cpu', 'tests/10.full.turns.cpu'], 'tests/10.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/10.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full11(self):
        naval = self.process([self.naval] + ['tests/11.full.rules', 'tests/11.full.map.plr', 'tests/11.full.map.cpu', 'tests/11.full.turns.cpu'], 'tests/11.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/11.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full12(self):
        naval = self.process([self.naval] + ['tests/12.full.rules', 'tests/12.full.map.plr', 'tests/12.full.map.cpu', 'tests/12.full.turns.cpu'], 'tests/12.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/12.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full13(self):
        naval = self.process([self.naval] + ['tests/13.full.rules', 'tests/13.full.map.plr', 'tests/13.full.map.cpu', 'tests/13.full.turns.cpu'], 'tests/13.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/13.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full14(self):
        naval = self.process([self.naval] + ['tests/14.full.rules', 'tests/14.full.map.plr', 'tests/14.full.map.cpu', 'tests/14.full.turns.cpu'], 'tests/14.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/14.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full15(self):
        naval = self.process([self.naval] + ['tests/15.full.rules', 'tests/15.full.map.plr', 'tests/15.full.map.cpu', 'tests/15.full.turns.cpu'], 'tests/15.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/15.full.out')
        self.assert_stderr_matches_file(naval, 'tests/badguess.err')
        self.assert_exit_status(naval, 130)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full16(self):
        naval = self.process([self.naval] + ['tests/16.full.rules', 'tests/16.full.map.plr', 'tests/16.full.map.cpu', 'tests/16.full.turns.cpu'], 'tests/16.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/16.full.out')
        self.assert_stderr_matches_file(naval, 'tests/giveup.err')
        self.assert_exit_status(naval, 140)

    @marks.marks('4-rules-wholegame', category_marks=6)
    def test_full17(self):
        naval = self.process([self.naval] + ['tests/17.full.rules', 'tests/17.full.map.plr', 'tests/17.full.map.cpu', 'tests/17.full.turns.cpu'], 'tests/17.full.turns.plr')
        naval.finish_input()
        self.assert_stdout_matches_file(naval, 'tests/17.full.out')
        self.assert_stderr_matches_file(naval, 'tests/empty')
        self.assert_exit_status(naval, 0)

if __name__=='__main__':
    marks.main()

