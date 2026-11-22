#!/usr/bin/env python3
"""Ejecuta pruebas de lógica C++ sin Arduino, red ni dispositivos."""
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
compiler = shutil.which('c++')
if not compiler:
    raise SystemExit('Se necesita un compilador c++ con soporte C++11.')
sources = sorted((root / 'firmware/tests').rglob('*_test.cpp'))
with tempfile.TemporaryDirectory(prefix='appmichi-tests-') as temporary:
    for index, source in enumerate(sources):
        binary = Path(temporary) / f'test-{index}'
        subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        print(f'OK: {source.relative_to(root)}', flush=True)
print(f'{len(sources)} programas de prueba completados.')
