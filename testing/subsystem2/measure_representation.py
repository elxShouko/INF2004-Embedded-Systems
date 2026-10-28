"""Measure existing application layouts without linking or executing firmware.

Uses target-compiler array symbols, not host sizeof, to measure ARM layout.
The field inventory is a declaration inventory; semantic bounds require review.
"""
from pathlib import Path
import argparse
from datetime import datetime, timezone
import hashlib
import json
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
ARM = Path('C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin')


def declarations():
    headers = sorted((ROOT / 'app_program/subsystem2').glob('*.h'))
    headers += [ROOT / 'app_program' / name for name in
                ('vehicle.h', 'vehicle_rt.h', 'gpio_bank.h')]
    records = []
    enums = {}
    for path in headers:
        original = path.read_text()
        clean = re.sub(r'/\*.*?\*/|//[^\n]*', '', original, flags=re.S)
        for match in re.finditer(r'typedef\s+enum\s*\{([^{}]*)\}\s*(\w+)\s*;', clean):
            values = [value.strip().split('=')[0].strip()
                      for value in match[1].split(',') if value.strip()]
            # Current declarations use consecutive implicit enum values.
            if any('=' in value and not re.search(r'=\s*0\s*$', value.strip())
                   for value in match[1].split(',') if value.strip()):
                raise SystemExit('Review explicit enum assignments: ' + match[2])
            enums[match[2]] = {'values': values,
                                'bits': max(1, (len(values) - 1).bit_length())}
        for match in re.finditer(r'typedef\s+(?:struct|union)\s*\{([^{}]*)\}\s*(\w+)\s*;', clean):
            fields = []
            for statement in match[1].split(';'):
                statement = ' '.join(statement.split())
                if not statement:
                    continue
                pointer = re.search(r'\(\s*\*\s*(\w+)\s*\)', statement)
                bitfield = re.fullmatch(r'(.+?)\s+(\w+)\s*:\s*(\d+)', statement)
                scalar = re.search(r'(\w+)\s*((?:\[[^]]+\]\s*)*)$', statement)
                if bitfield:
                    fields.append({'name': bitfield[2], 'declaration': statement,
                                   'kind': 'bitfield', 'declared_bits': int(bitfield[3])})
                    continue
                elif pointer:
                    name, kind = pointer[1], 'callback'
                elif scalar:
                    name, kind = scalar[1], 'field'
                else:
                    raise SystemExit('Unparsed declaration: ' + statement)
                fields.append({'name': name, 'declaration': statement, 'kind': kind})
            records.append({'type': match[2],
                            'source': path.relative_to(ROOT).as_posix(),
                            'fields': fields})
    for record in records:
        for field in record['fields']:
            declared = field['declaration'].split()[0]
            if declared == 'bool':
                field['logical_bits_per_element'] = 1
                field['range_basis'] = 'false/true; preserve current ownership'
            elif declared in enums:
                field['logical_bits_per_element'] = enums[declared]['bits']
                field['range_basis'] = 'current enum values; invalid-input checks remain required'
            elif field['kind'] == 'callback' or '*' in field['declaration']:
                field['range_basis'] = 'ABI pointer/callback; interpretation clarification needed'
            else:
                field['range_basis'] = 'review units/range/resolution/lifetime; no narrowing authorised by inventory'
    return headers, records, enums


def run(command, log):
    log.append(subprocess.list2cmdline(list(map(str, command))))
    result = subprocess.run(command, cwd=ROOT / 'build_make', capture_output=True,
                            text=True, timeout=120)
    log.append(result.stdout + result.stderr + f'EXIT {result.returncode}')
    if result.returncode:
        raise RuntimeError(log[-1])
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--label', required=True)
    opts = parser.parse_args()
    if not re.fullmatch(r'[A-Za-z0-9_]+', opts.label):
        parser.error('label must use letters, numbers and underscores')
    out = HERE / 'logs' / opts.label
    out.mkdir(exist_ok=False)
    headers, records, enums = declarations()
    log = []
    metadata = {'head': subprocess.check_output(['git', 'rev-parse', 'HEAD'],
                    cwd=ROOT, text=True).strip(),
                'status': subprocess.check_output(['git', 'status', '--short'],
                    cwd=ROOT, text=True),
                'utc': datetime.now(timezone.utc).isoformat(),
                'headers_sha256': {p.relative_to(ROOT).as_posix():
                    hashlib.sha256(p.read_bytes()).hexdigest() for p in headers},
                'measurement_script_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
    try:
        metadata['compiler'] = run([ARM / 'arm-none-eabi-gcc.exe', '--version'], log)
        # Reuse the recorded firmware include/ABI options, never invoke Make.
        recipe = next(line for line in (HERE / 'm2_make_recipes.txt').read_text().splitlines()
                      if line.startswith('arm-none-eabi-gcc '))
        includes = '#include <stddef.h>\n#define PROHIBIT_DEF_SIZE_T 1\n'
        includes += '#include "vehicle_rt.h"\n#include "gpio_bank.h"\n'
        includes += '#include "subsystem2/motion_bus.h"\n'
        includes += '#include "subsystem2/subsystem2.h"\n'
        includes += '#include "subsystem2/drv_rp2040.h"\n'
        includes += '#include "subsystem2/motion_foundation_test.h"\n'
        includes += '#include "subsystem2/test_drive.h"\n#include "subsystem2/test_motor.h"\n'
        scratch_root = HERE / 'build'
        scratch_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='layout_', dir=scratch_root) as tmp:
            scratch = Path(tmp)
            source, obj = scratch / 'layout.c', scratch / 'layout.o'
            lines = [includes]
            for record in records:
                name = record['type']
                lines.append(f'const unsigned char size__{name}[sizeof({name})] = {{0}};')
                for field in record['fields']:
                    if field['kind'] == 'bitfield':
                        continue  # C forbids sizeof/offsetof on a bit-field.
                    member = field['name']
                    lines.append(f'const unsigned char offset__{name}__{member}[offsetof({name}, {member}) + 1U] = {{0}};')
                    lines.append(f'const unsigned char field__{name}__{member}[sizeof((({name} *)0)->{member})] = {{0}};')
            source.write_text('\n'.join(lines) + '\n')
            (out / 'probe.c.txt').write_text(source.read_text())
            args = shlex.split(recipe)
            args[0] = str(ARM / 'arm-none-eabi-gcc.exe')
            args.insert(1, '-I' + str(ROOT / 'app_program'))
            for i, arg in enumerate(args):
                if arg.startswith('-MF'):
                    args[i] = '-MF' + str(scratch / 'layout.d')
                if arg.startswith('-MT'):
                    args[i] = '-MT' + str(obj)
                if arg == '-o':
                    args[i + 1] = str(obj)
            args[-1] = str(source)
            run(args, log)
            symbols = run([ARM / 'arm-none-eabi-nm.exe', '-S', str(obj)], log)
            sizes = {}
            for line in symbols.splitlines():
                parts = line.split()
                if len(parts) == 4:
                    sizes[parts[3]] = int(parts[1], 16)
            for record in records:
                name = record['type']
                record['arm_bytes'] = sizes['size__' + name]
                for field in record['fields']:
                    if field['kind'] == 'bitfield':
                        field['arm_offset'] = None
                        field['arm_bytes'] = None
                        continue
                    field['arm_offset'] = sizes[f'offset__{name}__{field["name"]}'] - 1
                    field['arm_bytes'] = sizes[f'field__{name}__{field["name"]}']
        # Existing ELF sizes are archival measurements, NOT rebuilt evidence.
        metadata['existing_elf_sections'] = []
        for elf in sorted((ROOT / 'build_make').glob('*.elf')):
            metadata['existing_elf_sections'].append({'path': elf.relative_to(ROOT).as_posix(),
                'sha256': hashlib.sha256(elf.read_bytes()).hexdigest(),
                'provenance': 'existing artifact; not rebuilt by this tool',
                'size_output': run([ARM / 'arm-none-eabi-size.exe', str(elf)], log)})
        metadata['records'] = records
        metadata['enums'] = enums
        (out / 'layout.json').write_text(json.dumps(metadata, indent=2) + '\n')
        md = ['# ARM application representation inventory', '',
              'Baseline ' + metadata['head'] + '. Declaration inventory, not a compliance certificate.',
              'Numeric ranges and calibration are unresolved unless explicitly justified.', '',
              '## Target structure sizes', '', '| Type | ARM bytes | Source |', '|---|---:|---|']
        md += [f'| `{r["type"]}` | {r["arm_bytes"]} | `{r["source"]}` |' for r in records]
        for record in records:
            md += ['', '## ' + record['type'], '',
                   '| Declaration | Offset | Bytes | Logical bits per element | Range basis |',
                   '|---|---:|---:|---:|---|']
            for field in record['fields']:
                md.append(f'| `{field["declaration"]}` | {field["arm_offset"]} | {field["arm_bytes"]} | '
                          f'{field.get("logical_bits_per_element", "TBD")} | {field["range_basis"]} |')
        (out / 'inventory.md').write_text('\n'.join(md) + '\n')
        print(f'Measured {len(records)} target structures and {sum(len(r["fields"]) for r in records)} declarations: {out}')
    finally:
        (out / 'commands.log').write_text('\n'.join(log) + '\n')


if __name__ == '__main__':
    main()
