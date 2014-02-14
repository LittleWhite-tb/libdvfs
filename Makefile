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
CFLAGS=-O3 -g -Wall -Wextra -fPIC

all: libdvfs.so freqdomain

.PHONY: all clean distclean install uninstall test doc

libdvfs.so: dvfs_core.o dvfs_unit.o dvfs_context.o
	$(CC) -shared $(CFLAGS) $^ -o $@

test: test_core test_cpu

test_core: test_core.o libdvfs.so
	$(CC) $(CFLAGS) $^ -o $@
	
test_cpu: test_cpu.o libdvfs.so
	$(CC) $(CFLAGS) $^ -o $@

freqdomain: freqdomain.o dvfs_core.o dvfs_unit.o dvfs_context.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

doc:
	doxygen libdvfs.doxy

install:
	/usr/bin/install -m 0755 libdvfs.so /usr/local/lib
	mkdir -p /usr/local/include/libdvfs
	/usr/bin/install -m 0655 dvfs_core.h /usr/local/include/libdvfs
	/usr/bin/install -m 0655 dvfs_context.h /usr/local/include/libdvfs
	/usr/bin/install -m 0655 dvfs_unit.h /usr/local/include/libdvfs
	/usr/bin/install -m 0655 libdvfs.h /usr/local/include/libdvfs

uninstall:
	rm -f /usr/local/lib/libdvfs.so
	rm -rf /usr/local/include/libdvfs

clean:
	rm -f *.o *.so

distclean: clean
	rm -f *.so
	rm -f test
	rm -r ./doc
