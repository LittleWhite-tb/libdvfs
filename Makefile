# libdvfs - A light library to set CPU governor and frequency
# Copyright (C) 2013 Universite de Versailles
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC?=gcc
CFLAGS=-O3 -g -Wall -Wextra

all: libdvfs.a

.PHONY:all clean distclean

libdvfs.a: core.o cpu.o
	ar rcs $@ $^

test: test_core test_cpu

test_core: test_core.o libdvfs.a
	$(CC) $(CFLAGS) $^ -o $@
	
test_cpu: test_cpu.o libdvfs.a
	$(CC) $(CFLAGS) $^ -o $@
	

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

doc:
	doxygen libdvfs.doxy

clean:
	rm -f *.o

distclean: clean
	rm -f *.a
	rm -f test
	rm -r ./doc
